#include "ASTDecoratedPrinter.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

// Constructor 

ASTDecoratedPrinter::ASTDecoratedPrinter(
    const std::vector<TabEntry>&  tab,
    const std::vector<BtabEntry>& btab,
    const std::vector<AtabEntry>& atab,
    std::ofstream& outFile
) : _tab(tab), _btab(btab), _atab(atab), _out(outFile) {}

// Emit / indent

void ASTDecoratedPrinter::emit(const std::string& s) {
    std::cout << s << "\n";
    if (_out.is_open()) _out << s << "\n";
}

std::string ASTDecoratedPrinter::ind(int d) const {
    return std::string(d * 2, ' ');
}

// Name helpers

std::string ASTDecoratedPrinter::typeFull(int t) const {
    switch (t) {
        case TYPE_VOID:       return "void";
        case TYPE_INTEGER:    return "integer";
        case TYPE_REAL:       return "real";
        case TYPE_BOOLEAN:    return "boolean";
        case TYPE_CHAR:       return "char";
        case TYPE_STRING:     return "string";
        case TYPE_ARRAY:      return "array";
        case TYPE_RECORD:     return "record";
        case TYPE_SUBRANGE:   return "subrange";
        case TYPE_ENUMERATED: return "enumerated";
        default:              return "unknown";
    }
}

std::string ASTDecoratedPrinter::typeShort(int t) const {
    switch (t) {
        case TYPE_VOID:       return "NOTYP";
        case TYPE_INTEGER:    return "INT";
        case TYPE_REAL:       return "REAL";
        case TYPE_BOOLEAN:    return "BOOL";
        case TYPE_CHAR:       return "CHAR";
        case TYPE_STRING:     return "STR";
        case TYPE_ARRAY:      return "ARRAY";
        case TYPE_RECORD:     return "RECORD";
        case TYPE_SUBRANGE:   return "SUBRANGE";
        case TYPE_ENUMERATED: return "ENUM";
        default:              return "UNKNOWN";
    }
}

std::string ASTDecoratedPrinter::objClass(int o) const {
    switch (o) {
        case OBJ_CONST:    return "CONST";
        case OBJ_VAR:      return "VAR";
        case OBJ_TYPE:     return "TYPE";
        case OBJ_PROC:     return "PROC";
        case OBJ_FUNC:     return "FUNC";
        case OBJ_PROGRAM:  return "PROGRAM";
        case OBJ_RESERVED: return "RESERVED";
        default:           return "?";
    }
}

std::string ASTDecoratedPrinter::opSym(const std::string& tok) const {
    if (tok == "plus")   return "+";
    if (tok == "minus")  return "-";
    if (tok == "times")  return "*";
    if (tok == "rdiv")   return "/";
    if (tok == "idiv")   return "div";
    if (tok == "imod")   return "mod";
    if (tok == "andsy")  return "and";
    if (tok == "orsy")   return "or";
    if (tok == "eql")    return "==";
    if (tok == "neq")    return "<>";
    if (tok == "lss")    return "<";
    if (tok == "leq")    return "<=";
    if (tok == "gtr")    return ">";
    if (tok == "geq")    return ">=";
    return tok;
}

int ASTDecoratedPrinter::findSymbol(const std::string& name, int objKind) const {
    // Cari dari belakang supaya kalau ada shadowing, yang paling baru yang kepakai
    for (int i = static_cast<int>(_tab.size()) - 1; i >= 0; --i) {
        if (_tab[i].id != name) continue;
        if (objKind != -1 && _tab[i].obj != objKind) continue;
        return i;
    }
    return -1;
}

std::string ASTDecoratedPrinter::inlineVarRef(ASTNode* n) const {
    // Bikin nama variabel lebih manusiawi buat output AST: a[1], r.a, matrix[i][j], dst
    if (!n) return "?";
    if (n->isTerminal) return n->tokenValue.empty() ? "?" : n->tokenValue;

    std::string result;
    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "ident" && result.empty()) {
            result = c->tokenValue;
            continue;
        }

        if (c->isTerminal || c->label != "<component-variable>") continue;

        bool isIndex = false;
        bool isField = false;
        for (auto* cc : c->children) {
            if (cc->isTerminal && cc->tokenType == "lbrack") isIndex = true;
            if (cc->isTerminal && cc->tokenType == "period") isField = true;
        }

        if (isIndex) {
            std::vector<std::string> idxParts;
            ASTNode* idxList = childNT(c, "<index-list>");
            if (idxList) {
                for (auto* idx : idxList->children) {
                    if (idx->isTerminal && idx->tokenType != "comma") {
                        idxParts.push_back(idx->tokenValue);
                    } else if (!idx->isTerminal) {
                        idxParts.push_back(inlineExpr(idx));
                    }
                }
            }
            result += "[";
            for (size_t i = 0; i < idxParts.size(); ++i) {
                if (i) result += ", ";
                result += idxParts[i];
            }
            result += "]";
        } else if (isField) {
            ASTNode* fld = childTerm(c, "ident");
            if (fld) result += "." + fld->tokenValue;
        }
    }

    return result.empty() ? "?" : result;
}

std::string ASTDecoratedPrinter::inlineProcCall(ASTNode* n) const {
    if (!n) return "?";
    std::string name;
    std::vector<std::string> args;

    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "ident" && name.empty()) {
            name = c->tokenValue;
        } else if (!c->isTerminal && c->label == "<parameter-list>") {
            for (auto* arg : c->children) {
                if (!arg->isTerminal && arg->label == "<expression>") {
                    args.push_back(inlineExpr(arg));
                }
            }
        }
    }

    std::string result = name.empty() ? "?" : name;
    result += "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) result += ", ";
        result += args[i];
    }
    result += ")";
    return result;
}

std::string ASTDecoratedPrinter::inlineExpr(ASTNode* n) const {
    if (!n) return "?";
    if (n->isTerminal) {
        if (n->tokenType == "intcon"  || n->tokenType == "realcon" ||
            n->tokenType == "charcon" || n->tokenType == "string"  ||
            n->tokenType == "ident")
            return n->tokenValue;
        return opSym(n->tokenType);
    }

    const std::string& lbl = n->label;
    if (lbl == "<variable>") return inlineVarRef(n);
    if (lbl == "<procedure/function-call>") return inlineProcCall(n);

    if (lbl == "<factor>") {
        bool hasNot = false, hasMinus = false;
        for (auto* c : n->children) {
            if (c->isTerminal && c->tokenType == "notsy")  hasNot   = true;
            if (c->isTerminal && c->tokenType == "minus")  hasMinus = true;
        }
        for (auto* c : n->children) {
            if (c->isTerminal && (c->tokenType == "notsy" || c->tokenType == "minus")) continue;
            std::string inner = inlineExpr(c);
            if (hasNot)   return "not " + inner;
            if (hasMinus) return "-"    + inner;
            return inner;
        }
        return "?";
    }

    if (lbl == "<index-list>") {
        std::string result;
        for (auto* c : n->children) {
            if (c->isTerminal && c->tokenType == "comma") result += ", ";
            else result += inlineExpr(c);
        }
        return result.empty() ? "?" : result;
    }

    std::string result;
    for (auto* c : n->children) {
        if (!c->isTerminal) result += inlineExpr(c);
        else if (c->tokenType != "lparent" && c->tokenType != "rparent") result += opSym(c->tokenType);
    }
    return result.empty() ? "?" : result;
}

bool ASTDecoratedPrinter::isRelOp(const std::string& tok) const {
    return tok=="eql"||tok=="neq"||tok=="lss"||tok=="leq"||tok=="gtr"||tok=="geq";
}
bool ASTDecoratedPrinter::isAddOp(const std::string& tok) const {
    return tok=="plus"||tok=="minus"||tok=="orsy";
}
bool ASTDecoratedPrinter::isMulOp(const std::string& tok) const {
    return tok=="times"||tok=="rdiv"||tok=="idiv"||tok=="imod"||tok=="andsy";
}

// Parse-tree navigation 

ASTNode* ASTDecoratedPrinter::childNT(ASTNode* n, const std::string& label) const {
    if (!n) return nullptr;
    for (auto* c : n->children)
        if (!c->isTerminal && c->label == label) return c;
    return nullptr;
}

ASTNode* ASTDecoratedPrinter::childTerm(ASTNode* n, const std::string& tok) const {
    if (!n) return nullptr;
    for (auto* c : n->children)
        if (c->isTerminal && c->tokenType == tok) return c;
    return nullptr;
}

std::string ASTDecoratedPrinter::progName(ASTNode* programNode) const {
    ASTNode* hdr = childNT(programNode, "<program-header>");
    if (hdr) {
        ASTNode* id = childTerm(hdr, "ident");
        if (id) return id->tokenValue;
    }
    return "main";
}

std::string ASTDecoratedPrinter::stripQuotes(const std::string& s) const {
    if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'')
        return s.substr(1, s.size() - 2);
    return s;
}

// Main entry

void ASTDecoratedPrinter::printAll(ASTNode* root) {
    emit("\nDECORATED AST:");
    emit("----------------------------------------");
    if (root) {
        doProg(root, 0);
    }
    else      emit("(empty program)");
    emit("----------------------------------------");

    doTab();
    doBtab();
    if (!_atab.empty()) doAtab();
}

// Decorated AST: program 

void ASTDecoratedPrinter::doProg(ASTNode* n, int d) {
    std::string name = progName(n);
    std::string line = ind(d) + "ProgramNode(name: '" + name + "'";
    if (n->tabIdx >= 0) line += ", tab_index: " + std::to_string(n->tabIdx);
    line += ")";
    emit(line);

    ASTNode* declPart = childNT(n, "<declaration-part>");
    ASTNode* compStmt = childNT(n, "<compound-statement>");

    if (declPart && !declPart->children.empty()) {
        emit(ind(d+1) + "Declarations:");
        doDecls(declPart, d+2);
    }
    if (compStmt) {
        doCompound(compStmt, d+1);
    }
}

// Declarations

void ASTDecoratedPrinter::doDecls(ASTNode* n, int d) {
    for (auto* c : n->children) {
        if (c->isTerminal) continue;
        if      (c->label == "<const-declaration>")     doConstDecl(c, d);
        else if (c->label == "<type-declaration>")      doTypeDecl(c, d);
        else if (c->label == "<var-declaration>")       doVarDecl(c, d);
        else if (c->label == "<subprogram-declaration>") doSubprog(c, d);
    }
}

void ASTDecoratedPrinter::doVarDecl(ASTNode* n, int d, bool isParam) {
    int ci = 0;
    if (ci < (int)n->children.size() && n->children[ci]->isTerminal &&
        n->children[ci]->tokenType == "varsy") ci++;

    while (ci < (int)n->children.size()) {
        ASTNode* identList = n->children[ci];
        if (!identList || identList->label != "<identifier-list>") break;
        ci++;                    
        if (ci < (int)n->children.size() && n->children[ci]->isTerminal) ci++; 
        ASTNode* typeNode = (ci < (int)n->children.size()) ? n->children[ci] : nullptr;
        if (typeNode && !typeNode->isTerminal) ci++; else { if (typeNode) ci++; }
        if (ci < (int)n->children.size() && n->children[ci]->isTerminal) ci++; 

        std::string tname = typeNode ? typeFull(typeNode->semType) : "unknown";

        for (auto* h : identList->children) {
            if (!h->isTerminal || h->tokenType != "ident") continue;
            std::string line = ind(d) + "VarDecl(name: '" + h->tokenValue + "'";
            line += ", type: " + tname;
            if (h->tabIdx >= 0) line += ", tab_index: " + std::to_string(h->tabIdx);
            line += ", level: " + std::to_string(h->lexLevel);
            if (isParam) line += ", parameter";
            line += ")";
            emit(line);
        }
    }
}

void ASTDecoratedPrinter::doConstDecl(ASTNode* n, int d) {
    int ci = 1; // skip constsy
    while (ci < (int)n->children.size()) {
        ASTNode* identNode = n->children[ci];
        if (!identNode || !identNode->isTerminal || identNode->tokenType != "ident") break;
        std::string name  = identNode->tokenValue;
        int tabIdx        = identNode->tabIdx;
        ci++;   
        ci++; 
        ASTNode* constN   = (ci < (int)n->children.size()) ? n->children[ci] : nullptr;
        if (constN && !constN->isTerminal) ci++; // <constant>
        if (ci < (int)n->children.size() && n->children[ci]->isTerminal) ci++; 

        std::string tname = constN ? typeFull(constN->semType) : "unknown";
        std::string line  = ind(d) + "ConstDecl(name: '" + name + "'";
        line += ", type: " + tname;
        if (tabIdx >= 0) line += ", tab_index: " + std::to_string(tabIdx);
        line += ")";
        emit(line);

        if (constN) {
            emit(ind(d+1) + "Value:");
            doConstantNode(constN, d+2);
        }
    }
}

void ASTDecoratedPrinter::doTypeDecl(ASTNode* n, int d) {
    int ci = 1;
    while (ci < (int)n->children.size()) {
        ASTNode* identNode = n->children[ci];
        if (!identNode || !identNode->isTerminal || identNode->tokenType != "ident") break;
        std::string name = identNode->tokenValue;
        int tabIdx       = identNode->tabIdx;
        ci++;  // ident
        ci++; // eql
        ASTNode* typeN   = (ci < (int)n->children.size()) ? n->children[ci] : nullptr;
        if (typeN && !typeN->isTerminal) ci++;
        if (ci < (int)n->children.size() && n->children[ci]->isTerminal) ci++;

        std::string kind = typeN ? typeFull(typeN->semType) : "unknown";
        std::string line = ind(d) + "TypeDecl(name: '" + name + "'";
        line += ", type: " + kind;
        if (tabIdx >= 0) line += ", tab_index: " + std::to_string(tabIdx);
        int blockIdx = (tabIdx >= 0 && tabIdx < (int)_tab.size()) ? _tab[tabIdx].ref : 0;
        if (kind == "record" && blockIdx > 0) {
            line += ", block_index: " + std::to_string(blockIdx);
        }
        line += ")";
        emit(line);

        if (kind == "record" && blockIdx > 0) {
            emit(ind(d+1) + "RecordType(type: record, block_index: " + std::to_string(blockIdx) + ")");
            doRecordFields(blockIdx, d+2);
        }
    }
}

void ASTDecoratedPrinter::doRecordFields(int blockIdx, int d) {
    if (blockIdx <= 0 || blockIdx >= (int)_btab.size()) return;

    std::vector<int> fields;
    int i = _btab[blockIdx].last;
    while (i > 0 && i < (int)_tab.size() && _tab[i].lev > 0) {
        if (_tab[i].obj == OBJ_VAR) fields.push_back(i);
        i = _tab[i].link;
    }

    for (auto it = fields.rbegin(); it != fields.rend(); ++it) {
        const TabEntry& e = _tab[*it];
        std::string line = ind(d) + "VarDecl(name: '" + e.id + "'";
        line += ", type: " + typeFull(e.type);
        line += ", tab_index: " + std::to_string(*it);
        line += ", level: " + std::to_string(e.lev);
        line += ")";
        emit(line);
    }
}

void ASTDecoratedPrinter::doSubprog(ASTNode* n, int d) {
    for (auto* c : n->children) {
        if (!c->isTerminal) {
            if (c->label == "<procedure-declaration>") doProcDecl(c, d);
            else if (c->label == "<function-declaration>")  doFuncDecl(c, d);
        }
    }
}

void ASTDecoratedPrinter::doProcDecl(ASTNode* n, int d) {
    std::string name;
    int tabIdx = -1;
    ASTNode* params = nullptr;
    ASTNode* block  = nullptr;
    bool seenName   = false;

    for (auto* c : n->children) {
        if (!seenName && c->isTerminal && c->tokenType == "ident") {
            name = c->tokenValue; tabIdx = c->tabIdx; seenName = true;
        }
        if (!c->isTerminal && c->label == "<formal-parameter-list>") params = c;
        if (!c->isTerminal && c->label == "<block>")                 block  = c;
    }

    std::string line = ind(d) + "ProcedureDecl(name: '" + name + "'";
    if (tabIdx >= 0) line += ", tab_index: " + std::to_string(tabIdx);
    line += ")";
    emit(line);

    if (params) {
        emit(ind(d+1) + "Parameters:");
        for (auto* c : params->children)
            if (!c->isTerminal && c->label == "<parameter-group>"){
                doVarDecl(c, d+2, true);  
            }
    }
    if (block) doBlock(block, d);
}

void ASTDecoratedPrinter::doFuncDecl(ASTNode* n, int d) {
    std::string name;
    int tabIdx   = -1;
    int retType  = TYPE_VOID;
    ASTNode* params = nullptr;
    ASTNode* block  = nullptr;
    bool seenName   = false;

    for (auto* c : n->children) {
        if (!seenName && c->isTerminal && c->tokenType == "ident") {
            name = c->tokenValue;
            tabIdx = c->tabIdx;
            seenName = true;
        }
        if (!c->isTerminal && c->label == "<formal-parameter-list>") params = c;
        if (!c->isTerminal && c->label == "<block>")                 block  = c;
    }

    // Kalau node terminal belum punya tabIdx, ambil dari symbol table
    // Ini bikin return_type tampil bener, misalnya integer bukan void
    if (tabIdx < 0 && !name.empty()) tabIdx = findSymbol(name, OBJ_FUNC);
    if (tabIdx >= 0 && tabIdx < static_cast<int>(_tab.size())) retType = _tab[tabIdx].type;

    std::string line = ind(d) + "FunctionDecl(name: '" + name + "'";
    line += ", return_type: " + typeFull(retType);
    if (tabIdx >= 0) line += ", tab_index: " + std::to_string(tabIdx);
    line += ")";
    emit(line);

    if (params) {
        emit(ind(d+1) + "Parameters:");
        for (auto* c : params->children)
            if (!c->isTerminal && c->label == "<parameter-group>")
                doVarDecl(c, d+2, true);
    }
    if (block) doBlock(block, d);
}

void ASTDecoratedPrinter::doBlock(ASTNode* n, int d) {
    ASTNode* declPart = childNT(n, "<declaration-part>");
    ASTNode* compStmt = childNT(n, "<compound-statement>");
    if (declPart && !declPart->children.empty()) {
        emit(ind(d+1) + "Declarations:");
        doDecls(declPart, d+2);
    }
    if (compStmt) {
        doCompound(compStmt, d+1);
    }
}

// Compound statement & statement list

void ASTDecoratedPrinter::doCompound(ASTNode* n, int d) {
    ASTNode* sl = childNT(n, "<statement-list>");
    if (n->tabIdx >= 0) {
        emit(ind(d) + "Block(block_index: " + std::to_string(n->tabIdx) +
             ", lev: " + std::to_string(n->lexLevel) + ")");
        if (sl) doStmtList(sl, d+1);
    } else {
        if (sl) doStmtList(sl, d);
    }
}

void ASTDecoratedPrinter::doStmtList(ASTNode* n, int d) {
    for (auto* c : n->children)
        if (!c->isTerminal) doStmt(c, d);
}

void ASTDecoratedPrinter::doStmt(ASTNode* n, int d) {
    if (!n) return;
    const std::string& lbl = n->label;
    if      (lbl == "<assignment-statement>")    doAssign(n, d);
    else if (lbl == "<if-statement>")            doIf(n, d);
    else if (lbl == "<while-statement>")         doWhile(n, d);
    else if (lbl == "<for-statement>")           doFor(n, d);
    else if (lbl == "<repeat-statement>")        doRepeat(n, d);
    else if (lbl == "<case-statement>")          doCase(n, d);
    else if (lbl == "<procedure/function-call>") doProcCall(n, d);
    else if (lbl == "<compound-statement>")      doCompound(n, d);
}

// Statements

void ASTDecoratedPrinter::doAssign(ASTNode* n, int d) {
    ASTNode* lhs    = nullptr;
    ASTNode* rhs    = nullptr;
    bool seenBecomes = false;

    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "becomes") { seenBecomes = true; continue; }
        if (!seenBecomes) lhs = c;
        else if (!rhs)    rhs = c;
    }

    std::string lhsName = "?";
    if (lhs) lhsName = lhs->isTerminal ? lhs->tokenValue : inlineVarRef(lhs);
    std::string rhsStr  = rhs ? inlineExpr(rhs) : "?";
    emit(ind(d) + "Assign('" + lhsName + "' := " + rhsStr + ", type: void)");

    // Print Target
    emit(ind(d+1) + "Target:");
    if (lhs) {
        if (lhs->isTerminal && lhs->tokenType == "ident") {
            std::string line = ind(d+2) + "Var(name: '" + lhs->tokenValue + "'";
            if (lhs->tabIdx  >= 0)         line += ", tab_index: " + std::to_string(lhs->tabIdx);
            if (lhs->semType != TYPE_VOID) line += ", type: " + typeFull(lhs->semType);
            if (lhs->lexLevel > 0)         line += ", level: " + std::to_string(lhs->lexLevel);
            line += ")";
            emit(line);
        } else if (!lhs->isTerminal) {
            doVarRef(lhs, d+2);
        }
    }

    // Print Value
    emit(ind(d+1) + "Value:");
    if (rhs) doExpr(rhs, d+2);
}

void ASTDecoratedPrinter::doIf(ASTNode* n, int d) {
    emit(ind(d) + "If(type: void)");
    ASTNode* cond     = nullptr;
    ASTNode* thenStmt = nullptr;
    ASTNode* elseStmt = nullptr;
    bool seenThen = false, seenElse = false;

    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "thensy") { seenThen = true; continue; }
        if (c->isTerminal && c->tokenType == "elsesy") { seenElse = true; continue; }
        if (c->isTerminal) continue;
        if (!seenThen && c->label == "<expression>") { cond = c; }
        else if (seenThen && !seenElse && !thenStmt) { thenStmt = c; }
        else if (seenElse && !elseStmt)              { elseStmt = c; }
    }

    if (cond)     { emit(ind(d+1) + "Condition:"); doExpr(cond, d+2); }
    if (thenStmt) { emit(ind(d+1) + "Then:");      doStmt(thenStmt, d+2); }
    if (elseStmt) { emit(ind(d+1) + "Else:");      doStmt(elseStmt, d+2); }
}

void ASTDecoratedPrinter::doWhile(ASTNode* n, int d) {
    emit(ind(d) + "While(type: void)");
    ASTNode* cond = childNT(n, "<expression>");
    ASTNode* body = childNT(n, "<compound-statement>");
    if (cond) { emit(ind(d+1) + "Condition:"); doExpr(cond, d+2); }
    if (body) { doCompound(body, d+1); }
}

void ASTDecoratedPrinter::doFor(ASTNode* n, int d) {
    std::string loopVar;
    bool isDownto = false;
    std::vector<ASTNode*> exprs;
    ASTNode* body = nullptr;

    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "ident" && loopVar.empty()) loopVar = c->tokenValue;
        if (c->isTerminal && c->tokenType == "downtosy") isDownto = true;
        if (!c->isTerminal && c->label == "<expression>") exprs.push_back(c);
        if (!c->isTerminal && c->label == "<compound-statement>") body = c;
    }

    emit(ind(d) + "For(variable: '" + loopVar + "', direction: " +
         (isDownto ? "downto" : "to") + ", type: void)");
    if (exprs.size() >= 1) { emit(ind(d+1) + "Start:"); doExpr(exprs[0], d+2); }
    if (exprs.size() >= 2) { emit(ind(d+1) + "End:");   doExpr(exprs[1], d+2); }
    if (body)              { doCompound(body, d+1); }
}

void ASTDecoratedPrinter::doRepeat(ASTNode* n, int d) {
    emit(ind(d) + "Repeat(type: void)");
    ASTNode* sl   = childNT(n, "<statement-list>");
    ASTNode* cond = nullptr;
    bool seenUntil = false;
    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "untilsy") { seenUntil = true; continue; }
        if (!c->isTerminal && c->label == "<expression>" && seenUntil) cond = c;
    }
    if (sl)   { emit(ind(d+1) + "Body:");  doStmtList(sl, d+2); }
    if (cond) { emit(ind(d+1) + "Until:"); doExpr(cond, d+2); }
}

void ASTDecoratedPrinter::doCase(ASTNode* n, int d) {
    emit(ind(d) + "Case(type: void)");
    ASTNode* expr = childNT(n, "<expression>");
    if (expr) { emit(ind(d+1) + "Expression:"); doExpr(expr, d+2); }
    emit(ind(d+1) + "Cases:");
    ASTNode* cb = childNT(n, "<case-block>");
    if (cb) doCaseBlock(cb, d+2);
}

void ASTDecoratedPrinter::doCaseBlock(ASTNode* n, int d) {
    bool printedColon = false;
    for (auto* c : n->children) {
        if (c->isTerminal) continue;
        if (c->label == "<constant>") {
            // Print the case label value
            for (auto* cc : c->children) {
                if (cc->isTerminal) {
                    emit(ind(d) + "Case " + cc->tokenValue + ":");
                    printedColon = true;
                    break;
                }
            }
        } else if (c->label == "<case-block>") {
            doCaseBlock(c, d);
        } else {
            // The statement
            emit(ind(d+1) + "Statement:");
            doStmt(c, d+2);
            printedColon = false;
        }
    }
    (void)printedColon;
}

void ASTDecoratedPrinter::doProcCall(ASTNode* n, int d) {
    std::string name;
    int tabIdx = n ? n->tabIdx : -1;
    int callType = n ? n->semType : TYPE_VOID;
    ASTNode* paramList = nullptr;

    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "ident" && name.empty()) {
            name = c->tokenValue;
            if (tabIdx < 0) tabIdx = c->tabIdx;
        }
        if (!c->isTerminal && c->label == "<parameter-list>") paramList = c;
    }

    // Built-in / function call kadang belum bawa tabIdx di node terminal,
    // jadi fallback ke symbol table biar type output lebih akurat
    if (tabIdx < 0 && !name.empty()) tabIdx = findSymbol(name);
    if (callType == TYPE_VOID && tabIdx >= 0 && tabIdx < static_cast<int>(_tab.size()) && _tab[tabIdx].obj == OBJ_FUNC) {
        callType = _tab[tabIdx].type;
    }

    std::string line = ind(d) + "ProcCall(name: '" + name + "'";
    if (tabIdx >= 0) line += ", tab_index: " + std::to_string(tabIdx);
    line += ", type: " + typeFull(callType) + ")";
    emit(line);

    if (paramList) {
        emit(ind(d+1) + "Arguments:");
        for (auto* c : paramList->children)
            if (!c->isTerminal && c->label == "<expression>") doExpr(c, d+2);
    }
}

// Expressions

void ASTDecoratedPrinter::doExpr(ASTNode* n, int d) {
    if (!n) return;
    const std::string& lbl = n->label;

    if (lbl == "<expression>") {
        std::vector<ASTNode*> sExprs;
        std::string relOp;
        for (auto* c : n->children) {
            if (!c->isTerminal && c->label == "<simple-expression>") sExprs.push_back(c);
            else if (c->isTerminal && isRelOp(c->tokenType)) relOp = c->tokenType;
        }
        if (!relOp.empty() && sExprs.size() >= 2) {
            emit(ind(d) + "BinOp(op: '" + opSym(relOp) + "', type: boolean)");
            emit(ind(d+1) + "Left:");  doExpr(sExprs[0], d+2);
            emit(ind(d+1) + "Right:"); doExpr(sExprs[1], d+2);
        } else if (!sExprs.empty()) {
            doExpr(sExprs[0], d);
        }

    } else if (lbl == "<simple-expression>") {
        std::vector<ASTNode*> terms;
        std::vector<std::string> ops;
        bool negUnary = false;

        for (auto* c : n->children) {
            if (terms.empty() && c->isTerminal && c->tokenType == "minus") { negUnary = true; }
            else if (!c->isTerminal && c->label == "<term>") terms.push_back(c);
            else if (c->isTerminal && isAddOp(c->tokenType)) ops.push_back(c->tokenType);
        }
        if (terms.size() == 1 && !negUnary) {
            doExpr(terms[0], d);
        } else if (!terms.empty()) {
            if (negUnary && terms.size() == 1) {
                emit(ind(d) + "UnaryOp(op: '-', type: " + typeFull(n->semType) + ")");
                emit(ind(d+1) + "Operand:");
                doExpr(terms[0], d+2);
            } else {
                doBinChain(terms, ops, n->semType, d);
            }
        }

    } else if (lbl == "<term>") {
        std::vector<ASTNode*> factors;
        std::vector<std::string> ops;
        for (auto* c : n->children) {
            if (!c->isTerminal && c->label == "<factor>") factors.push_back(c);
            else if (c->isTerminal && isMulOp(c->tokenType)) ops.push_back(c->tokenType);
        }
        if (factors.size() == 1) doExpr(factors[0], d);
        else                     doBinChain(factors, ops, n->semType, d);

    } else if (lbl == "<factor>") {
        doFactor(n, d);

    } else if (lbl == "<constant>") {
        doConstantNode(n, d);
    }
}

void ASTDecoratedPrinter::doBinChain(
    const std::vector<ASTNode*>& operands,
    const std::vector<std::string>& opToks,
    int resultType,
    int d)
{
    if (operands.empty()) return;
    if (operands.size() == 1) { doExpr(operands[0], d); return; }

    std::string op = opToks.empty() ? "+" : opSym(opToks[0]);
    emit(ind(d) + "BinOp(op: '" + op + "', type: " + typeFull(resultType) + ")");

    if (operands.size() == 2) {
        emit(ind(d+1) + "Left:");  doExpr(operands[0], d+2);
        emit(ind(d+1) + "Right:"); doExpr(operands[1], d+2);
    } else {
        // Left-fold: (... ((a op b) op c) op d ...)
        std::vector<ASTNode*>    leftOps(operands.begin(), operands.end()-1);
        std::vector<std::string> leftToks(opToks.empty() ? opToks
                                          : std::vector<std::string>(opToks.begin(), opToks.end()-1));
        emit(ind(d+1) + "Left:");
        doBinChain(leftOps, leftToks, resultType, d+2);
        emit(ind(d+1) + "Right:");
        doExpr(operands.back(), d+2);
    }
}

void ASTDecoratedPrinter::doFactor(ASTNode* n, int d) {
    if (n->children.empty()) return;
    ASTNode* first = n->children[0];

    if (first->isTerminal) {
        if (first->tokenType == "intcon") {
            emit(ind(d) + "Number(int: " + first->tokenValue + ", type: integer)");

        } else if (first->tokenType == "realcon") {
            emit(ind(d) + "Number(real: " + first->tokenValue + ", type: real)");

        } else if (first->tokenType == "charcon") {
            emit(ind(d) + "Char(value: " + first->tokenValue + ")");

        } else if (first->tokenType == "string") {
            emit(ind(d) + "String(value: \"" + stripQuotes(first->tokenValue) + "\")");

        } else if (first->tokenType == "notsy") {
            emit(ind(d) + "UnaryOp(op: 'not', type: boolean)");
            emit(ind(d+1) + "Operand:");
            if (n->children.size() >= 2) doFactor(n->children[1], d+2);

        } else if (first->tokenType == "lparent") {
            // ( expression )
            for (auto* c : n->children)
                if (!c->isTerminal && c->label == "<expression>") { doExpr(c, d); return; }

        } else if (first->tokenType == "ident") {
            std::string line = ind(d) + "Var(name: '" + first->tokenValue + "'";
            if (first->tabIdx  >= 0)         line += ", tab_index: " + std::to_string(first->tabIdx);
            if (first->semType != TYPE_VOID) line += ", type: " + typeFull(first->semType);
            if (first->lexLevel > 0)         line += ", level: " + std::to_string(first->lexLevel);
            line += ")";
            emit(line);
        }

    } else {
        if      (first->label == "<procedure/function-call>") doProcCall(first, d);
        else if (first->label == "<variable>")                doVarRef(first, d);
        else                                                  doExpr(first, d);
    }
}

void ASTDecoratedPrinter::doVarRef(ASTNode* n, int d) {
    if (!n) return;
    if (n->isTerminal && n->tokenType == "ident") {
        std::string line = ind(d) + "Var(name: '" + n->tokenValue + "'";
        if (n->tabIdx  >= 0)         line += ", tab_index: " + std::to_string(n->tabIdx);
        if (n->semType != TYPE_VOID) line += ", type: " + typeFull(n->semType);
        if (n->lexLevel > 0)         line += ", level: " + std::to_string(n->lexLevel);
        line += ")";
        emit(line);
        return;
    }
    std::string name;
    int tabIdx = -1, stype = TYPE_UNKNOWN, slev = 0;
    for (auto* c : n->children) {
        if (c->isTerminal && c->tokenType == "ident") {
            name = c->tokenValue; tabIdx = c->tabIdx; stype = c->semType; slev = c->lexLevel;
            break;
        }
    }
    std::string line = ind(d) + "Var(name: '" + name + "'";
    if (tabIdx >= 0)             line += ", tab_index: " + std::to_string(tabIdx);
    if (stype  != TYPE_UNKNOWN)  line += ", type: " + typeFull(n->semType > 0 ? n->semType : stype);
    if (slev   > 0)              line += ", level: " + std::to_string(slev);
    line += ")";
    emit(line);

    for (auto* c : n->children) {
        if (c->isTerminal || c->label != "<component-variable>") continue;
        for (auto* cc : c->children) {
            if (cc->isTerminal && cc->tokenType == "lbrack") {
                ASTNode* idxList = childNT(c, "<index-list>");
                if (idxList && !idxList->children.empty()) {
                    emit(ind(d+1) + "[Index]");
                }
            } else if (cc->isTerminal && cc->tokenType == "period") {
                ASTNode* fld = childTerm(c, "ident");
                if (fld) emit(ind(d+1) + ".field: " + fld->tokenValue);
            }
        }
    }
}

void ASTDecoratedPrinter::doConstantNode(ASTNode* n, int d) {
    bool neg = false;
    for (auto* c : n->children) {
        if (!c->isTerminal) continue;
        if (c->tokenType == "minus") { neg = true; continue; }
        if (c->tokenType == "plus")  { continue; }
        if (c->tokenType == "intcon") {
            emit(ind(d) + "Number(int: " + (neg ? "-" : "") + c->tokenValue + ", type: integer)");
        } else if (c->tokenType == "realcon") {
            emit(ind(d) + "Number(real: " + (neg ? "-" : "") + c->tokenValue + ", type: real)");
        } else if (c->tokenType == "charcon") {
            emit(ind(d) + "Char(value: " + c->tokenValue + ")");
        } else if (c->tokenType == "string") {
            emit(ind(d) + "String(value: \"" + stripQuotes(c->tokenValue) + "\")");
        } else if (c->tokenType == "ident") {
            std::string line = ind(d) + "Var(name: '" + c->tokenValue + "'";
            if (c->tabIdx  >= 0)         line += ", tab_index: " + std::to_string(c->tabIdx);
            if (c->semType != TYPE_VOID) line += ", type: " + typeFull(c->semType);
            line += ")";
            emit(line);
        }
        return; 
    }
}

// Symbol tables

void ASTDecoratedPrinter::doTab() {
    emit("\n=== TAB (Identifier Table) ===");
    emit("  Idx  Identifier        Link   Obj        Type      Ref  Nrm  Lev  Adr");
    emit("  -------------------------------------------------------------------------");

    for (int i = 0; i < (int)_tab.size(); i++) {
        std::ostringstream oss;
        oss << "  "
            << std::setw(3) << i << "  "
            << std::left  << std::setw(17) << _tab[i].id
            << std::right << std::setw(5)  << _tab[i].link << "  "
            << std::left  << std::setw(10) << objClass(_tab[i].obj)
            << std::setw(10) << typeShort(_tab[i].type)
            << std::right << std::setw(4)  << _tab[i].ref
            << std::setw(5)  << _tab[i].nrm
            << std::setw(5)  << _tab[i].lev
            << std::setw(5)  << _tab[i].adr;
        emit(oss.str());
    }
    emit("  -------------------------------------------------------------------------");
}

void ASTDecoratedPrinter::doBtab() {
    emit("\n=== BTAB (Block Table) ===");
    emit("  Idx  Last  Lpar  Psze  Vsze");
    emit("  ----------------------------------------");
    for (int i = 0; i < (int)_btab.size(); i++) {
        std::ostringstream oss;
        oss << "  "
            << std::setw(3) << i << "  "
            << std::setw(4) << _btab[i].last << " "
            << std::setw(5) << _btab[i].lpar << " "
            << std::setw(5) << _btab[i].psze << " "
            << std::setw(5) << _btab[i].vsze;
        emit(oss.str());
    }
    emit("  ----------------------------------------");
}

void ASTDecoratedPrinter::doAtab() {
    emit("\n=== ATAB (Array Table) ===");
    emit("  Idx  XTyp      ETyp      Eref  Low   High  Elsz  Size");
    emit("  ---------------------------------------------------------");
    for (int i = 0; i < (int)_atab.size(); i++) {
        std::ostringstream oss;
        oss << "  "
            << std::setw(3) << (i+1) << "  "
            << std::left << std::setw(10) << typeShort(_atab[i].xtyp)
            << std::setw(10) << typeShort(_atab[i].etyp)
            << std::right
            << std::setw(5) << _atab[i].eref
            << std::setw(6) << _atab[i].low
            << std::setw(6) << _atab[i].high
            << std::setw(6) << _atab[i].elsz
            << "  " << _atab[i].size;
        emit(oss.str());
    }
}
