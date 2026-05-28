#include "IntermediateCodeGenerator.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

IntermediateCodeGenerator::IntermediateCodeGenerator(
    const std::vector<TabEntry>& t,
    const std::vector<BtabEntry>& b,
    const std::vector<AtabEntry>& a)
    : tab(t), btab(b), atab(a) {}

// Entry point utama: reset state, siapkan layout, generate IC, lalu opsional tulis ke file
Code IntermediateCodeGenerator::generate(ASTNode* root, const std::string& outFilename) {
    code.clear();
    locations.clear();
    subprograms.clear();
    subprogramOrder.clear();
    globalSize = 0;
    currentBlockId = 0;
    currentFunctionTabIdx = -1;

    if (!root) return code;
    prepareLayouts(root);
    genProgram(root);
    if (!outFilename.empty()) writeToFile(outFilename);
    return code;
}

// Tambah satu instruksi ke vector code dan balikin index-nya, berguna buat backpatch jump
int IntermediateCodeGenerator::emit(const std::string& op, int level, int arg,
                                    const std::string& literal, const std::string& comment) {
    code.emplace_back(op, level, arg, literal, comment);
    return static_cast<int>(code.size()) - 1;
}

// Backpatch target jump yang belum diketahui saat instruksi dibuat
void IntermediateCodeGenerator::patchArg(int instructionIndex, int newArg) {
    if (instructionIndex < 0 || instructionIndex >= static_cast<int>(code.size()))
        throw std::runtime_error("invalid patch index");
    code[instructionIndex].arg = newArg;
}

void IntermediateCodeGenerator::writeToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) throw std::runtime_error("cannot open IC output: " + filename);
    for (int i = 0; i < static_cast<int>(code.size()); ++i) out << code[i].toString(i, true) << '\n';
}

// Ubah string menjadi huruf kecil untuk pencarian case‑insensitive
std::string IntermediateCodeGenerator::lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// Periksa apakah node adalah terminal dengan tipe token tertentu
bool IntermediateCodeGenerator::isTerminal(ASTNode* n, const std::string& tokenType) {
    return n && n->isTerminal && n->tokenType == tokenType;
}

// Ambil child pertama node yang non‑terminal dengan label tertentu
ASTNode* IntermediateCodeGenerator::childNT(ASTNode* n, const std::string& label) const {
    if (!n) return nullptr;
    for (auto* c : n->children) if (c && !c->isTerminal && c->label == label) return c;
    return nullptr;
}

// Ambil child pertama node yang terminal dengan tipe token tertentu
ASTNode* IntermediateCodeGenerator::childTerm(ASTNode* n, const std::string& tokenType) const {
    if (!n) return nullptr;
    for (auto* c : n->children) if (isTerminal(c, tokenType)) return c;
    return nullptr;
}

// Ambil semua child non‑terminal dengan label tertentu
std::vector<ASTNode*> IntermediateCodeGenerator::childrenNT(ASTNode* n, const std::string& label) const {
    std::vector<ASTNode*> res;
    if (!n) return res;
    for (auto* c : n->children) if (c && !c->isTerminal && c->label == label) res.push_back(c);
    return res;
}

// Ambil semua node <expression> dari children node
std::vector<ASTNode*> IntermediateCodeGenerator::expressions(ASTNode* n) const {
    return childrenNT(n, "<expression>");
}

// Hitung ukuran memori (dalam slot) untuk suatu tipe data
int IntermediateCodeGenerator::typeSize(int type, int ref) const {
    switch (type) {
        case TYPE_INTEGER: case TYPE_REAL: case TYPE_BOOLEAN: case TYPE_CHAR: case TYPE_STRING:
        case TYPE_SUBRANGE: case TYPE_ENUMERATED: return 1;
        case TYPE_ARRAY:
            if (ref > 0 && ref <= static_cast<int>(atab.size())) return std::max(1, atab[ref - 1].size);
            return 1;
        case TYPE_RECORD:
            if (ref > 0 && ref < static_cast<int>(btab.size())) return std::max(1, btab[ref].vsze);
            return 1;
        default: return 1;
    }
}

// Hitung ukuran memori dari entri symbol table berdasarkan indeksnya
int IntermediateCodeGenerator::symbolSize(int tabIdx) const {
    if (tabIdx < 0 || tabIdx >= static_cast<int>(tab.size())) return 1;
    return typeSize(tab[tabIdx].type, tab[tabIdx].ref);
}

// Cari indeks symbol table berdasarkan nama dan (opsional) jenis objek
int IntermediateCodeGenerator::findSymbol(const std::string& name, int objKind) const {
    const std::string want = lower(name);
    for (int i = static_cast<int>(tab.size()) - 1; i >= 0; --i) {
        if (lower(tab[i].id) == want && (objKind < 0 || tab[i].obj == objKind)) return i;
    }
    return -1;
}

// Hapus tanda petik tunggal di awal dan akhir string (untuk char/string literal)
std::string IntermediateCodeGenerator::stripQuotes(const std::string& s) const {
    if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'') return s.substr(1, s.size() - 2);
    return s;
}

// Ubah node terminal (intcon, realcon, charcon, string, atau konstanta boolean) menjadi representasi literal untuk instruksi LIT
std::string IntermediateCodeGenerator::makeLiteral(ASTNode* terminal) const {
    if (!terminal) return "0";
    const std::string& tok = terminal->tokenType;
    const std::string& val = terminal->tokenValue;
    if (tok == "string") return "\"" + stripQuotes(val) + "\"";
    if (tok == "charcon") return "'" + stripQuotes(val) + "'";
    if (tok == "ident" && terminal->tabIdx >= 0 && terminal->tabIdx < static_cast<int>(tab.size()) && tab[terminal->tabIdx].obj == OBJ_CONST) {
        if (tab[terminal->tabIdx].type == TYPE_BOOLEAN) return tab[terminal->tabIdx].adr ? "true" : "false";
        return std::to_string(tab[terminal->tabIdx].adr);
    }
    return val.empty() ? "0" : val;
}

// Konversi tipe token (misal "plus", "eql") ke kode operasi OPR yang sesuai
int IntermediateCodeGenerator::opFromToken(const std::string& tokenType) const {
    if (tokenType == "plus") return OPR_ADD;
    if (tokenType == "minus") return OPR_SUB;
    if (tokenType == "times") return OPR_MUL;
    if (tokenType == "rdiv" || tokenType == "idiv") return OPR_DIV;
    if (tokenType == "imod") return OPR_MOD;
    if (tokenType == "eql") return OPR_EQL;
    if (tokenType == "neq") return OPR_NEQ;
    if (tokenType == "lss") return OPR_LSS;
    if (tokenType == "leq") return OPR_LEQ;
    if (tokenType == "gtr") return OPR_GTR;
    if (tokenType == "geq") return OPR_GEQ;
    if (tokenType == "andsy") return OPR_AND;
    if (tokenType == "orsy") return OPR_OR;
    throw std::runtime_error("unsupported operator token: " + tokenType);
}

// Kumpulkan alamat global, procedure/function, parameter, dan variabel lokal sebelum codegen
void IntermediateCodeGenerator::prepareLayouts(ASTNode* root) {
    ASTNode* declPart = childNT(root, "<declaration-part>");
    collectGlobalVars(declPart);
    collectSubprograms(declPart);
    for (int idx : subprogramOrder) buildSubprogramLayout(subprograms[idx]);
}

// Variabel global ditempatkan di blockId 0, offset-nya naik sesuai ukuran tipe
void IntermediateCodeGenerator::collectGlobalVars(ASTNode* declPart) {
    if (!declPart) return;
    for (auto* d : declPart->children) {
        if (!d || d->isTerminal || d->label != "<var-declaration>") continue;
        for (auto* idList : childrenNT(d, "<identifier-list>")) {
            for (auto* id : idList->children) if (isTerminal(id, "ident")) addVarLocationFromIdent(id, 0, globalSize);
        }
    }
}

// Catat semua procedure/function supaya CALL tahu lompat ke instruksi mana
void IntermediateCodeGenerator::collectSubprograms(ASTNode* declPart) {
    if (!declPart) return;
    for (auto* subDeclWrap : declPart->children) {
        if (!subDeclWrap || subDeclWrap->isTerminal || subDeclWrap->label != "<subprogram-declaration>") continue;
        ASTNode* decl = nullptr;
        bool isFunc = false;
        for (auto* c : subDeclWrap->children) {
            if (!c || c->isTerminal) continue;
            if (c->label == "<procedure-declaration>") { decl = c; isFunc = false; }
            if (c->label == "<function-declaration>") { decl = c; isFunc = true; }
        }
        if (!decl) continue;
        ASTNode* nameNode = nullptr;
        for (auto* c : decl->children) if (isTerminal(c, "ident")) { nameNode = c; break; }
        if (!nameNode) continue;
        int sym = findSymbol(nameNode->tokenValue, isFunc ? OBJ_FUNC : OBJ_PROC);
        if (sym < 0) continue;
        SubprogramInfo info;
        info.tabIdx = sym;
        info.blockId = tab[sym].ref;
        info.isFunction = isFunc;
        info.declaration = decl;
        info.block = childNT(decl, "<block>");
        info.name = tab[sym].id;
        subprograms[sym] = info;
        subprogramOrder.push_back(sym);
        if (info.block) collectSubprograms(childNT(info.block, "<declaration-part>"));
    }
}

// Susun frame procedure/function: parameter dulu, lalu variabel lokal
void IntermediateCodeGenerator::buildSubprogramLayout(SubprogramInfo& info) {
    int nextOffset = info.isFunction ? 1 : 0; // function pakai slot 0 buat nilai balik
    ASTNode* formal = childNT(info.declaration, "<formal-parameter-list>");
    if (formal) {
        for (auto* group : childrenNT(formal, "<parameter-group>")) {
            ASTNode* idList = childNT(group, "<identifier-list>");
            if (!idList) continue;
            for (auto* id : idList->children) {
                if (isTerminal(id, "ident")) {
                    addVarLocationFromIdent(id, info.blockId, nextOffset);
                    info.paramCount++;
                }
            }
        }
    }

    ASTNode* declPart = info.block ? childNT(info.block, "<declaration-part>") : nullptr;
    if (declPart) {
        for (auto* d : declPart->children) {
            if (!d || d->isTerminal || d->label != "<var-declaration>") continue;
            for (auto* idList : childrenNT(d, "<identifier-list>"))
                for (auto* id : idList->children)
                    if (isTerminal(id, "ident")) addVarLocationFromIdent(id, info.blockId, nextOffset);
        }
    }
    info.frameSize = std::max(nextOffset, info.isFunction ? 1 : 0);
}

// Daftarkan lokasi memori sebuah identifier dari symbol table ke layout runtime
void IntermediateCodeGenerator::addVarLocationFromIdent(ASTNode* ident, int blockId, int& nextOffset) {
    if (!ident || !ident->isTerminal) return;
    int idx = ident->tabIdx;
    if (idx < 0) idx = findSymbol(ident->tokenValue, OBJ_VAR);
    if (idx < 0 || idx >= static_cast<int>(tab.size())) return;
    if (locations.count(idx)) return;
    VarLocation loc;
    loc.blockId = blockId;
    loc.offset = nextOffset;
    loc.size = symbolSize(idx);
    loc.type = tab[idx].type;
    loc.ref = tab[idx].ref;
    locations[idx] = loc;
    nextOffset += std::max(1, loc.size);
}

// Alokasikan slot temporary di memori global (untuk menyimpan nilai sementara)
int IntermediateCodeGenerator::allocTemp() {
    int offset = globalSize++;
    return offset;
}

// Program utama: alokasikan global, skip body subprogram, lalu jalankan main compound statement
void IntermediateCodeGenerator::genProgram(ASTNode* node) {
    int intIndex = emit("INT", 0, 0, "", "allocate global memory");
    int jumpMain = emit("JMP", 0, 0, "", "jump over subprogram bodies");

    ASTNode* declPart = childNT(node, "<declaration-part>");
    (void)declPart;
    for (int idx : subprogramOrder) genSubprogram(subprograms[idx]);

    int mainStart = static_cast<int>(code.size());
    patchArg(jumpMain, mainStart);
    currentBlockId = 0;
    currentFunctionTabIdx = -1;
    ASTNode* comp = childNT(node, "<compound-statement>");
    if (comp) genCompound(comp);
    emit("RET", 0, 0, "", "end program");
    patchArg(intIndex, globalSize);
}

// Body procedure/function ditaruh sebelum main dan hanya jalan saat dipanggil CAL
void IntermediateCodeGenerator::genSubprogram(const SubprogramInfo& info) {
    subprograms[info.tabIdx].entry = static_cast<int>(code.size());
    emit("INT", info.blockId, info.frameSize, "", "activate frame for " + info.name);
    int prevBlock = currentBlockId;
    int prevFunc = currentFunctionTabIdx;
    currentBlockId = info.blockId;
    currentFunctionTabIdx = info.isFunction ? info.tabIdx : -1;
    if (info.block) {
        ASTNode* declPart = childNT(info.block, "<declaration-part>");
        // Subprogram nested sudah di-emit di bagian awal; deklarasi tidak perlu dieksekusi
        (void)declPart;
        ASTNode* comp = childNT(info.block, "<compound-statement>");
        if (comp) genCompound(comp);
    }
    emit("RET", 0, 0, "", "return from " + info.name);
    currentBlockId = prevBlock;
    currentFunctionTabIdx = prevFunc;
}

// Generate kode untuk node compound-statement (blok begin-end)
void IntermediateCodeGenerator::genCompound(ASTNode* node) {
    ASTNode* sl = childNT(node, "<statement-list>");
    if (sl) genStatementList(sl);
}

// Dispatcher statement: pilih generator sesuai label node AST
void IntermediateCodeGenerator::genStatementList(ASTNode* node) {
    if (!node) return;
    for (auto* c : node->children) if (c && !c->isTerminal) genStatement(c);
}

void IntermediateCodeGenerator::genStatement(ASTNode* node) {
    if (!node) return;
    const std::string& lbl = node->label;
    if (lbl == "<assignment-statement>") genAssignment(node);
    else if (lbl == "<if-statement>") genIf(node);
    else if (lbl == "<case-statement>") genCase(node);
    else if (lbl == "<while-statement>") genWhile(node);
    else if (lbl == "<repeat-statement>") genRepeat(node);
    else if (lbl == "<for-statement>") genFor(node);
    else if (lbl == "<procedure/function-call>") genProcFuncCall(node, false);
    else if (lbl == "<compound-statement>") genCompound(node);
}

// Assignment: push alamat target dulu, hitung ekspresi, lalu STI untuk simpan indirect
void IntermediateCodeGenerator::genAssignment(ASTNode* node) {
    ASTNode* lhs = nullptr;
    ASTNode* rhs = nullptr;
    bool seenBecomes = false;
    for (auto* c : node->children) {
        if (isTerminal(c, "becomes")) { seenBecomes = true; continue; }
        if (!seenBecomes && c && (c->isTerminal || c->label == "<variable>")) lhs = c;
        else if (seenBecomes && c && !c->isTerminal && c->label == "<expression>") rhs = c;
    }
    if (!lhs || !rhs) return;
    genAddress(lhs);
    genExpression(rhs);
    emit("STI", 0, 0, "", "store assignment target");
}

// IF pakai pola fall-through: kondisi false lompat ke else/end
void IntermediateCodeGenerator::genIf(ASTNode* node) {
    ASTNode* cond = nullptr;
    ASTNode* thenStmt = nullptr;
    ASTNode* elseStmt = nullptr;
    bool seenThen = false, seenElse = false;
    for (auto* c : node->children) {
        if (isTerminal(c, "thensy")) { seenThen = true; continue; }
        if (isTerminal(c, "elsesy")) { seenElse = true; continue; }
        if (!c || c->isTerminal) continue;
        if (!seenThen && c->label == "<expression>") cond = c;
        else if (seenThen && !seenElse && !thenStmt) thenStmt = c;
        else if (seenElse && !elseStmt) elseStmt = c;
    }
    if (cond) genExpression(cond);
    int jpc = emit("JPC", 0, 0, "", "if false");
    if (thenStmt) genStatement(thenStmt);
    if (elseStmt) {
        int jmpEnd = emit("JMP", 0, 0, "", "skip else");
        patchArg(jpc, static_cast<int>(code.size()));
        genStatement(elseStmt);
        patchArg(jmpEnd, static_cast<int>(code.size()));
    } else {
        patchArg(jpc, static_cast<int>(code.size()));
    }
}

// Kumpulkan semua cabang case dari node case-block ke dalam vector of pairs (constants, statement)
void IntermediateCodeGenerator::collectCaseBranches(
    ASTNode* caseBlock, std::vector<std::pair<std::vector<ASTNode*>, ASTNode*>>& branches) const {
    if (!caseBlock) return;
    std::vector<ASTNode*> constants;
    ASTNode* stmt = nullptr;
    ASTNode* next = nullptr;
    bool afterColon = false;
    for (auto* c : caseBlock->children) {
        if (isTerminal(c, "colon")) { afterColon = true; continue; }
        if (c && !c->isTerminal && c->label == "<constant>" && !afterColon) constants.push_back(c);
        else if (c && !c->isTerminal && c->label == "<case-block>") next = c;
        else if (c && !c->isTerminal && afterColon && !stmt) stmt = c;
    }
    if (!constants.empty()) branches.push_back({constants, stmt});
    if (next) collectCaseBranches(next, branches);
}

// CASE: selector disimpan sementara, lalu tiap branch dibandingkan satu-satu
void IntermediateCodeGenerator::genCase(ASTNode* node) {
    ASTNode* selector = childNT(node, "<expression>");
    ASTNode* block = childNT(node, "<case-block>");
    if (!selector || !block) return;

    int selectorTemp = allocTemp();
    genExpression(selector);
    emit("STO", 0, selectorTemp, "", "save case selector");

    std::vector<std::pair<std::vector<ASTNode*>, ASTNode*>> branches;
    collectCaseBranches(block, branches);
    std::vector<int> endJumps;

    for (auto& branch : branches) {
        bool first = true;
        for (auto* constant : branch.first) {
            emit("LOD", 0, selectorTemp, "", "load case selector");
            genConstant(constant);
            emit("OPR", 0, OPR_EQL, "", "case equality test");
            if (!first) emit("OPR", 0, OPR_OR, "", "case multiple constants");
            first = false;
        }
        int jpcNext = emit("JPC", 0, 0, "", "case next branch");
        if (branch.second) genStatement(branch.second);
        endJumps.push_back(emit("JMP", 0, 0, "", "case end"));
        patchArg(jpcNext, static_cast<int>(code.size()));
    }
    for (int j : endJumps) patchArg(j, static_cast<int>(code.size()));
}

// WHILE: cek kondisi di awal, false keluar, true jalankan body lalu lompat balik
void IntermediateCodeGenerator::genWhile(ASTNode* node) {
    ASTNode* cond = childNT(node, "<expression>");
    ASTNode* body = childNT(node, "<compound-statement>");
    int start = static_cast<int>(code.size());
    if (cond) genExpression(cond);
    int jpc = emit("JPC", 0, 0, "", "while false");
    if (body) genCompound(body);
    emit("JMP", 0, start, "", "while repeat");
    patchArg(jpc, static_cast<int>(code.size()));
}

// REPEAT-UNTIL: body jalan minimal sekali, lalu ulang selama kondisi masih false
void IntermediateCodeGenerator::genRepeat(ASTNode* node) {
    ASTNode* stmtList = childNT(node, "<statement-list>");
    ASTNode* cond = nullptr;
    bool seenUntil = false;
    for (auto* c : node->children) {
        if (isTerminal(c, "untilsy")) { seenUntil = true; continue; }
        if (seenUntil && c && !c->isTerminal && c->label == "<expression>") cond = c;
    }
    int start = static_cast<int>(code.size());
    if (stmtList) genStatementList(stmtList);
    if (cond) genExpression(cond);
    emit("JPC", 0, start, "", "repeat until false");
}

// FOR: simpan nilai akhir di temporary global supaya batas loop stabil
void IntermediateCodeGenerator::genFor(ASTNode* node) {
    ASTNode* var = nullptr;
    std::vector<ASTNode*> exprs;
    ASTNode* body = nullptr;
    bool downto = false;
    for (auto* c : node->children) {
        if (isTerminal(c, "ident") && !var) var = c;
        if (isTerminal(c, "downtosy")) downto = true;
        if (c && !c->isTerminal && c->label == "<expression>") exprs.push_back(c);
        if (c && !c->isTerminal && c->label == "<compound-statement>") body = c;
    }
    if (!var || exprs.size() < 2) return;
    int endTemp = allocTemp();

    genAddress(var);
    genExpression(exprs[0]);
    emit("STI", 0, 0, "", "for initial value");
    genExpression(exprs[1]);
    emit("STO", 0, endTemp, "", "for final value");

    int start = static_cast<int>(code.size());
    genVariableValue(var);
    emit("LOD", 0, endTemp, "", "for bound");
    emit("OPR", 0, downto ? OPR_GEQ : OPR_LEQ, "", "for condition");
    int jpc = emit("JPC", 0, 0, "", "for done");
    if (body) genCompound(body);
    genAddress(var);
    genVariableValue(var);
    emit("LIT", 0, 1, "", "for step 1");
    emit("OPR", 0, downto ? OPR_SUB : OPR_ADD, "", "for increment/decrement");
    emit("STI", 0, 0, "", "update for variable");
    emit("JMP", 0, start, "", "for repeat");
    patchArg(jpc, static_cast<int>(code.size()));
}

// Panggilan built-in ditangani langsung; user-defined pakai CAL dan metadata frame
void IntermediateCodeGenerator::genProcFuncCall(ASTNode* node, bool keepFunctionResult) {
    ASTNode* name = childTerm(node, "ident");
    if (!name) return;
    std::string lname = lower(name->tokenValue);

    ASTNode* paramList = childNT(node, "<parameter-list>");
    std::vector<ASTNode*> args = paramList ? expressions(paramList) : std::vector<ASTNode*>{};

    if (lname == "writeln") {
        for (auto* e : args) { genExpression(e); emit("OPR", 0, OPR_WRITE, "", "writeln arg"); }
        emit("OPR", 0, OPR_NEWLINE, "", "writeln newline");
        return;
    }
    if (lname == "write") {
        for (auto* e : args) { genExpression(e); emit("OPR", 0, OPR_WRITE, "", "write arg"); }
        return;
    }
    if (lname == "readln") {
        if (paramList) {
            for (auto* c : paramList->children) {
                if (c && !c->isTerminal && c->label == "<expression>") {
                    // readln targetnya variabel. Kalau parser membungkusnya sebagai expression, kita turun pelan-pelan sampai ketemu variable/factor pertamanya
                    ASTNode* se = childNT(c, "<simple-expression>");
                    ASTNode* term = se ? childNT(se, "<term>") : nullptr;
                    ASTNode* factor = term ? childNT(term, "<factor>") : nullptr;
                    ASTNode* target = factor && !factor->children.empty() ? factor->children[0] : nullptr;
                    genAddress(target);
                    emit("OPR", 0, OPR_READLN, "", "readln value");
                    emit("STI", 0, 0, "", "readln store");
                }
            }
        }
        return;
    }

    int calleeIdx = name->tabIdx;
    if (calleeIdx < 0) calleeIdx = findSymbol(name->tokenValue);
    auto it = subprograms.find(calleeIdx);
    if (it == subprograms.end()) {
        std::cerr << "[IC WARNING] Unsupported procedure/function call: " << name->tokenValue << "\n";
        return;
    }
    SubprogramInfo& info = it->second;
    for (auto* e : args) genExpression(e);
    std::ostringstream meta;
    meta << "params=" << args.size() << ";size=" << info.frameSize << ";ret=" << (info.isFunction ? 1 : 0);
    emit("CAL", info.blockId, info.entry, meta.str(), "call " + info.name);
    if (info.isFunction && !keepFunctionResult) emit("POP", 0, 0, "", "discard unused function result");
}

// Expression menghasilkan satu nilai di puncak operand stack
void IntermediateCodeGenerator::genExpression(ASTNode* node) {
    if (!node) { emit("LIT", 0, 0); return; }
    std::vector<ASTNode*> terms = childrenNT(node, "<simple-expression>");
    ASTNode* rel = nullptr;
    for (auto* c : node->children) if (c && c->isTerminal && (c->tokenType == "eql" || c->tokenType == "neq" || c->tokenType == "gtr" || c->tokenType == "geq" || c->tokenType == "lss" || c->tokenType == "leq")) rel = c;
    if (terms.empty()) { emit("LIT", 0, 0); return; }
    genSimpleExpression(terms[0]);
    if (rel && terms.size() > 1) {
        genSimpleExpression(terms[1]);
        emit("OPR", 0, opFromToken(rel->tokenType), "", "relational op");
    }
}

// Simple expression menangani +, -, OR, termasuk unary minus di depan
void IntermediateCodeGenerator::genSimpleExpression(ASTNode* node) {
    if (!node) { emit("LIT", 0, 0); return; }
    int i = 0, n = static_cast<int>(node->children.size());
    bool unaryMinus = false;
    if (i < n && node->children[i]->isTerminal && (node->children[i]->tokenType == "plus" || node->children[i]->tokenType == "minus")) {
        unaryMinus = node->children[i]->tokenType == "minus";
        i++;
    }
    if (i >= n) { emit("LIT", 0, 0); return; }
    genTerm(node->children[i++]);
    if (unaryMinus) emit("OPR", 0, OPR_NEG, "", "unary minus");
    while (i < n) {
        ASTNode* op = node->children[i++];
        if (!op || !op->isTerminal) continue;
        if (i >= n) break;
        genTerm(node->children[i++]);
        emit("OPR", 0, opFromToken(op->tokenType), "", "simple-expression op");
    }
}

// Term menangani *, div, /, mod, dan AND
void IntermediateCodeGenerator::genTerm(ASTNode* node) {
    if (!node) { emit("LIT", 0, 0); return; }
    int i = 0, n = static_cast<int>(node->children.size());
    if (i >= n) { emit("LIT", 0, 0); return; }
    genFactor(node->children[i++]);
    while (i < n) {
        ASTNode* op = node->children[i++];
        if (!op || !op->isTerminal) continue;
        if (i >= n) break;
        genFactor(node->children[i++]);
        emit("OPR", 0, opFromToken(op->tokenType), "", "term op");
    }
}

// Factor adalah daun ekspresi: literal, variable, function call, NOT, atau kurung
void IntermediateCodeGenerator::genFactor(ASTNode* node) {
    if (!node || node->children.empty()) { emit("LIT", 0, 0); return; }
    ASTNode* first = node->children[0];
    if (isTerminal(first, "intcon") || isTerminal(first, "realcon") || isTerminal(first, "charcon") || isTerminal(first, "string")) {
        emit("LIT", 0, 0, makeLiteral(first), "literal");
        return;
    }
    if (isTerminal(first, "ident")) {
        if (first->tabIdx >= 0 && first->tabIdx < static_cast<int>(tab.size()) && tab[first->tabIdx].obj == OBJ_CONST) {
            emit("LIT", 0, 0, makeLiteral(first), "constant identifier");
        } else {
            genVariableValue(first);
        }
        return;
    }
    if (isTerminal(first, "notsy")) {
        if (node->children.size() > 1) genFactor(node->children[1]);
        emit("OPR", 0, OPR_NOT, "", "not");
        return;
    }
    if (isTerminal(first, "lparent")) {
        for (auto* c : node->children) if (c && !c->isTerminal && c->label == "<expression>") { genExpression(c); return; }
    }
    if (!first->isTerminal && first->label == "<variable>") { genVariableValue(first); return; }
    if (!first->isTerminal && first->label == "<procedure/function-call>") { genProcFuncCall(first, true); return; }
    emit("LIT", 0, 0, "", "unsupported factor fallback");
}

// Constant khusus dipakai untuk case label dan deklarasi konstanta
void IntermediateCodeGenerator::genConstant(ASTNode* node) {
    if (!node) { emit("LIT", 0, 0); return; }
    bool neg = false;
    for (auto* c : node->children) {
        if (isTerminal(c, "minus")) neg = true;
        if (isTerminal(c, "intcon") || isTerminal(c, "realcon") || isTerminal(c, "charcon") || isTerminal(c, "string") || isTerminal(c, "ident")) {
            emit("LIT", 0, 0, makeLiteral(c), "constant");
            if (neg) emit("OPR", 0, OPR_NEG, "", "negative constant");
            return;
        }
    }
    emit("LIT", 0, 0);
}

// Index-list di grammar bisa rekursif, jadi diratakan dulu biar gampang diproses
std::vector<ASTNode*> IntermediateCodeGenerator::flattenIndexList(ASTNode* indexList) const {
    std::vector<ASTNode*> res;
    if (!indexList) return res;
    for (auto* c : indexList->children) {
        if (!c) continue;
        if (c->isTerminal && (c->tokenType == "intcon" || c->tokenType == "charcon" || c->tokenType == "ident")) res.push_back(c);
        else if (!c->isTerminal && c->label == "<index-list>") {
            auto sub = flattenIndexList(c);
            res.insert(res.end(), sub.begin(), sub.end());
        }
    }
    return res;
}

// Index array bisa literal, konstanta, atau variabel; semuanya akhirnya push nilai index
void IntermediateCodeGenerator::genIndexValue(ASTNode* indexNode) {
    if (!indexNode) { emit("LIT", 0, 0); return; }
    if (isTerminal(indexNode, "intcon") || isTerminal(indexNode, "charcon")) emit("LIT", 0, 0, makeLiteral(indexNode), "array index");
    else if (isTerminal(indexNode, "ident")) {
        if (indexNode->tabIdx >= 0 && indexNode->tabIdx < static_cast<int>(tab.size()) && tab[indexNode->tabIdx].obj == OBJ_CONST)
            emit("LIT", 0, 0, makeLiteral(indexNode), "array const index");
        else genVariableValue(indexNode);
    } else emit("LIT", 0, 0);
}

// Cari lokasi runtime identifier. Nama function sendiri dianggap slot return value
IntermediateCodeGenerator::VarLocation IntermediateCodeGenerator::locationForIdent(ASTNode* ident) const {
    if (!ident || !ident->isTerminal) throw std::runtime_error("expected identifier for address generation");
    int idx = ident->tabIdx;
    if (idx < 0) idx = findSymbol(ident->tokenValue, OBJ_VAR);
    if (idx == currentFunctionTabIdx) return VarLocation{currentBlockId, 0, 1, tab[idx].type, tab[idx].ref};
    auto it = locations.find(idx);
    if (it != locations.end()) return it->second;
    if (idx >= 0 && idx < static_cast<int>(tab.size()) && tab[idx].obj == OBJ_VAR) return VarLocation{0, tab[idx].adr, symbolSize(idx), tab[idx].type, tab[idx].ref};
    throw std::runtime_error("unknown variable location: " + ident->tokenValue);
}

// Ambil offset field record dari btab/tab. Offset ini ditambahkan ke alamat record
int IntermediateCodeGenerator::fieldOffset(int recordRef, const std::string& fieldName, int* outType, int* outRef) const {
    if (recordRef <= 0 || recordRef >= static_cast<int>(btab.size())) throw std::runtime_error("invalid record reference");
    int i = btab[recordRef].last;
    std::string want = lower(fieldName);
    while (i > 0) {
        if (lower(tab[i].id) == want) {
            if (outType) *outType = tab[i].type;
            if (outRef) *outRef = tab[i].ref;
            return tab[i].adr;
        }
        i = tab[i].link;
    }
    throw std::runtime_error("unknown record field: " + fieldName);
}

// Generate alamat final untuk variable, termasuk arr[i] dan rec.field
void IntermediateCodeGenerator::genAddress(ASTNode* lhsOrVariable) {
    if (!lhsOrVariable) throw std::runtime_error("missing assignment target");
    ASTNode* varNode = lhsOrVariable;
    if (lhsOrVariable->isTerminal && lhsOrVariable->tokenType == "ident") {
        VarLocation loc = locationForIdent(lhsOrVariable);
        emit("LDA", loc.blockId, loc.offset, "", "address of " + lhsOrVariable->tokenValue);
        return;
    }
    if (varNode->label != "<variable>") throw std::runtime_error("assignment target is not a variable");

    ASTNode* ident = childTerm(varNode, "ident");
    VarLocation loc = locationForIdent(ident);
    emit("LDA", loc.blockId, loc.offset, "", "address of " + ident->tokenValue);
    int curType = loc.type;
    int curRef = loc.ref;

    for (auto* comp : varNode->children) {
        if (!comp || comp->isTerminal || comp->label != "<component-variable>") continue;
        if (childTerm(comp, "lbrack")) {
            auto idxs = flattenIndexList(childNT(comp, "<index-list>"));
            for (auto* idxNode : idxs) {
                if (curType != TYPE_ARRAY || curRef <= 0 || curRef > static_cast<int>(atab.size()))
                    throw std::runtime_error("indexing a non-array variable");
                const AtabEntry& arr = atab[curRef - 1];
                genIndexValue(idxNode);
                emit("CHK", arr.low, arr.high, "", "array bounds check");
                emit("LIT", 0, arr.low, "", "array low bound");
                emit("OPR", 0, OPR_SUB, "", "index - low");
                emit("LIT", 0, std::max(1, arr.elsz), "", "element size");
                emit("OPR", 0, OPR_MUL, "", "array offset");
                emit("OPR", 0, OPR_ADD, "", "array element address");
                curType = arr.etyp;
                curRef = arr.eref;
            }
        } else if (childTerm(comp, "period")) {
            ASTNode* field = childTerm(comp, "ident");
            int newType = TYPE_UNKNOWN, newRef = 0;
            int off = fieldOffset(curRef, field ? field->tokenValue : "", &newType, &newRef);
            emit("LIT", 0, off, "", "record field offset");
            emit("OPR", 0, OPR_ADD, "", "record field address");
            curType = newType;
            curRef = newRef;
        }
    }
}

// Nilai variabel = alamatnya dulu, lalu load indirect dengan LDI
void IntermediateCodeGenerator::genVariableValue(ASTNode* node) {
    genAddress(node);
    emit("LDI", 0, 0, "", "load indirect");
}
