#include "SemanticAnalyzer.hpp"
#include "ASTDecoratedPrinter.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdlib>

// Constructor / Destructor

SemanticAnalyzer::SemanticAnalyzer()
    : currentLevel(0), mainBlockIndex(-1), currentLine(0), hasError(false) {
    initPredefined();
}

SemanticAnalyzer::~SemanticAnalyzer() {
    if (outFile.is_open()) outFile.close();
}

// Utility

std::string SemanticAnalyzer::toLower(const std::string& s) const {
    std::string r = s;
    for (char& c : r) c = (char)std::tolower((unsigned char)c);
    return r;
}

std::string SemanticAnalyzer::typeCodeName(int t) const {
    switch (t) {
        case TYPE_VOID: return "void";
        case TYPE_INTEGER: return "integer";
        case TYPE_REAL: return "real";
        case TYPE_BOOLEAN: return "boolean";
        case TYPE_CHAR: return "char";
        case TYPE_STRING: return "string";
        case TYPE_ARRAY: return "array";
        case TYPE_RECORD: return "record";
        case TYPE_SUBRANGE: return "subrange";
        case TYPE_ENUMERATED: return "enumerated";
        default:  return "unknown";
    }
}

std::string SemanticAnalyzer::objKindName(int o) const {
    switch (o) {
        case OBJ_CONST:return "const";
        case OBJ_VAR: return "var";
        case OBJ_TYPE:  return "type";
        case OBJ_PROC: return "proc";
        case OBJ_FUNC: return "func";
        case OBJ_PROGRAM: return "prog";
        default: return "?";
    }
}

bool SemanticAnalyzer::isRelOp(const std::string& t) const {
    return t == "eql" || t == "neq" || t == "gtr" ||t == "geq" || t == "lss" || t == "leq";
}

bool SemanticAnalyzer::isNumeric(int t) const {
    return t == TYPE_INTEGER || t == TYPE_REAL;
}

// Output

void SemanticAnalyzer::emit(const std::string& s) {
    std::cout << s << "\n";
    if (outFile.is_open()) outFile << s << "\n";
}

void SemanticAnalyzer::semanticError(const std::string& msg) {
    std::string prefix = "[SEMANTIC ERROR]";
    if (currentLine > 0) prefix += " Line " + std::to_string(currentLine) + ":";
    std::cerr << prefix << " " << msg << "\n";
    if (outFile.is_open()) outFile << prefix << " " << msg << "\n";
    hasError = true;
}

void SemanticAnalyzer::semanticWarning(const std::string& msg) {
    std::cerr << "[SEMANTIC WARNING] " << msg << "\n";
}

int SemanticAnalyzer::getNodeLine(ASTNode* node) const {
    if (!node) return 0;
    if (node->line > 0) return node->line;
    for (auto* c : node->children) {
        int l = getNodeLine(c);
        if (l > 0) return l;
    }
    return 0;
}

bool SemanticAnalyzer::tryGetConstInt(ASTNode* node, int& val) const {
    if (!node) return false;
    if (node->isTerminal && node->tokenType == "intcon") {
        val = std::stoi(node->tokenValue);
        return true;
    }
    if (node->isTerminal) return false;
    if (node->children.size() == 1)
        return tryGetConstInt(node->children[0], val);
    if (node->children.size() == 2 &&
        node->children[0]->isTerminal && node->children[0]->tokenType == "minus") {
        if (tryGetConstInt(node->children[1], val)) { val = -val; return true; }
    }
    return false;
}


void SemanticAnalyzer::initPredefined() {
    tab.push_back({"", 0, OBJ_TYPE, TYPE_VOID, 0, 1, 0, 0});

    btab.push_back({0, 0, 0, 0});
    display.push_back(0);

    static const struct { const char* name; int typeCode; } rwords[32] = {
        {"and",       TYPE_VOID},  
        {"array",     TYPE_VOID},    // 2                       
        {"begin",     TYPE_VOID},    // 3
        {"case",      TYPE_VOID},    // 4                           
        {"const",     TYPE_VOID},    // 5
        {"div",       TYPE_VOID},    // 6
        {"downto",    TYPE_VOID},    // 7
        {"do",        TYPE_VOID},    // 8
        {"else",      TYPE_VOID},    // 9
        {"end",       TYPE_VOID},    // 10
        {"for",       TYPE_VOID},    // 11
        {"function",  TYPE_VOID},    // 12
        {"if",        TYPE_VOID},    // 13
        {"mod",       TYPE_VOID},    // 14
        {"not",       TYPE_VOID},    // 15
        {"of",        TYPE_VOID},    // 16
        {"or",        TYPE_VOID},    // 17
        {"procedure", TYPE_VOID},    // 18
        {"program",   TYPE_VOID},    // 19
        {"record",    TYPE_VOID},    // 20
        {"repeat",    TYPE_VOID},    // 21
        {"integer",   TYPE_INTEGER}, // 22 
        {"real",      TYPE_REAL},    // 23 
        {"boolean",   TYPE_BOOLEAN}, // 24 
        {"char",      TYPE_CHAR},    // 25 
        {"string",    TYPE_STRING},  // 26 
        {"then",      TYPE_VOID},    // 27
        {"to",        TYPE_VOID},    // 28
        {"type",      TYPE_VOID},    // 29
        {"until",     TYPE_VOID},    // 30
        {"var",       TYPE_VOID},    // 31
        {"while",     TYPE_VOID},    // 32
    };

    for (int i = 0; i < 32; i++) {
        TabEntry e;
        e.id   = rwords[i].name;
        e.link = i;
        e.obj  = (rwords[i].typeCode != TYPE_VOID) ? OBJ_TYPE : OBJ_RESERVED;
        e.type = rwords[i].typeCode;
        e.ref  = 0;
        e.nrm  = 1;
        e.lev  = 0;
        e.adr  = 0;
        tab.push_back(e);
    }
    btab[0].last = 32;

    auto addPre = [&](const char* name, int obj, int type, int adr) {
        TabEntry e;
        e.id   = name;
        e.link = btab[0].last;
        e.obj  = obj;
        e.type = type;
        e.ref  = 0;
        e.nrm  = 1;
        e.lev  = 0;
        e.adr  = adr;
        tab.push_back(e);
        btab[0].last = (int)tab.size() - 1;
    };

    addPre("true",    OBJ_CONST, TYPE_BOOLEAN, 1); 
    addPre("false",   OBJ_CONST, TYPE_BOOLEAN, 0); 
    addPre("writeln", OBJ_PROC,  TYPE_VOID,    0); 
    addPre("readln",  OBJ_PROC,  TYPE_VOID,    0); 
}

// Block management

int SemanticAnalyzer::ensureMainBlock() {
    if (mainBlockIndex >= 0) return mainBlockIndex;

    BtabEntry b;
    b.last = 0;
    b.lpar = 0;
    b.psze = 0;
    b.vsze = 0;
    btab.push_back(b);
    mainBlockIndex = (int)btab.size() - 1;
    return mainBlockIndex;
}

int SemanticAnalyzer::enterBlock() {
    BtabEntry b;
    b.last = btab[display[currentLevel]].last;
    b.lpar = 0;
    b.psze = 0;
    b.vsze = 0;
    btab.push_back(b);
    int newIdx = (int)btab.size() - 1;

    currentLevel++;
    if ((int)display.size() <= currentLevel)
        display.push_back(newIdx);
    else
        display[currentLevel] = newIdx;

    return newIdx;
}

void SemanticAnalyzer::leaveBlock() {
    if (currentLevel > 0) currentLevel--;
}


int SemanticAnalyzer::addIdentifier(const std::string& name, int obj, int type, int ref, int nrm, int adr) {
    std::string lname = toLower(name);
    int i = btab[display[currentLevel]].last;
    while (i > 0 && tab[i].lev >= currentLevel) {
        if (toLower(tab[i].id) == lname) {
            semanticError("Identifier '" + name + "' already declared in this scope");
            return i;
        }
        i = tab[i].link;
    }

    TabEntry e;
    e.id = name;
    e.link = btab[display[currentLevel]].last;
    e.obj = obj;
    e.type = type;
    e.ref = ref;
    e.nrm = nrm;
    e.lev = currentLevel;
    e.adr = adr;
    tab.push_back(e);

    int idx = (int)tab.size() - 1;
    btab[display[currentLevel]].last = idx;
    return idx;
}


int SemanticAnalyzer::lookupIdent(const std::string& name, bool errorIfMissing) {
    std::string lname = toLower(name);
    int i = btab[display[currentLevel]].last;
    while (i > 0) {
        if (toLower(tab[i].id) == lname) return i;
        i = tab[i].link;
    }
    if (errorIfMissing)
        semanticError("Identifier '" + name + "' is not declared");
    return -1;
}

int SemanticAnalyzer::resolveTypeName(const std::string& name) {
    int idx = lookupIdent(name, false);
    if (idx < 0) { semanticError("Unknown type '" + name + "'"); return TYPE_UNKNOWN; }
    if (tab[idx].obj != OBJ_TYPE) {
        semanticError("'" + name + "' is not a type");
        return TYPE_UNKNOWN;
    }
    return tab[idx].type;
}

int SemanticAnalyzer::getTypeSize(int typeCode, int ref) const {
    switch (typeCode) {
        case TYPE_INTEGER: case TYPE_REAL:
        case TYPE_BOOLEAN: case TYPE_CHAR:
        case TYPE_STRING:  return 1;
        case TYPE_ARRAY:
            if (ref > 0 && ref <= (int)atab.size()) return atab[ref-1].size;
            return 0;
        case TYPE_RECORD:
            if (ref > 0 && ref < (int)btab.size()) return btab[ref].vsze;
            return 0;
        default: return 1;
    }
}


bool SemanticAnalyzer::typesCompatible(int t1, int t2) const {
    if (t1 == t2) return true;
    // Integer and Real are mutually compatible for comparisons / arithmetic
    if (isNumeric(t1) && isNumeric(t2)) return true;
    return false;
}

bool SemanticAnalyzer::assignCompatible(int targetType, int valueType) const {
    if (targetType == valueType) return true;
    // Real := Integer is allowed
    if (targetType == TYPE_REAL && valueType == TYPE_INTEGER) return true;
    if (targetType == TYPE_SUBRANGE && valueType == TYPE_INTEGER) return true;
    if (targetType == TYPE_SUBRANGE && valueType == TYPE_SUBRANGE) return true;
    if (valueType == TYPE_UNKNOWN) return true;
    return false;
}


void SemanticAnalyzer::analyze(ASTNode* root, const std::string& outFilename) {
    outFile.open(outFilename);
    if (!root) { emit("(empty program)"); return; }

    visitProgram(root);

    ASTDecoratedPrinter printer(tab, btab, atab, outFile);
    printer.printAll(root);

    emit(hasError ? "\n[Semantic analysis completed with ERRORS]"
                  : "\n[Semantic analysis completed successfully]");
}


void SemanticAnalyzer::visitProgram(ASTNode* node) {
    node->lexLevel = 0;
    node->semType  = TYPE_VOID;

    ASTNode* header   = nullptr;
    ASTNode* declPart = nullptr;
    ASTNode* compStmt = nullptr;

    for (auto* c : node->children) {
        if (!c->isTerminal) {
            if      (c->label == "<program-header>")     header   = c;
            else if (c->label == "<declaration-part>")   declPart = c;
            else if (c->label == "<compound-statement>") compStmt = c;
        }
    }

    std::string progName = "main";
    if (header) {
        header->semType = TYPE_VOID;
        for (auto* c : header->children) {
            if (c->isTerminal && c->tokenType == "ident") {
                progName = c->tokenValue;
                c->semType = TYPE_VOID;
                break;
            }
        }
    }
    int progIdx = addIdentifier(progName, OBJ_PROGRAM, TYPE_VOID, 0, 1, 0);
    node->tabIdx = progIdx;

    if (declPart) visitDeclarationPart(declPart);
    int mainBlk = ensureMainBlock();
    if (compStmt) {
        compStmt->tabIdx   = mainBlk;
        compStmt->lexLevel = 1;
        visitCompoundStatement(compStmt);
    }
}


void SemanticAnalyzer::visitDeclarationPart(ASTNode* node) {
    for (auto* c : node->children) {
        if (c->isTerminal) continue;
        if      (c->label == "<const-declaration>")     visitConstDeclaration(c);
        else if (c->label == "<type-declaration>")      visitTypeDeclaration(c);
        else if (c->label == "<var-declaration>")       visitVarDeclaration(c);
        else if (c->label == "<subprogram-declaration>")visitSubprogramDeclaration(c);
    }
    node->semType = TYPE_VOID;
}


void SemanticAnalyzer::visitConstDeclaration(ASTNode* node) {
    int ci = 1; 
    while (ci < (int)node->children.size()) {
        if (node->children[ci]->tokenType != "ident") break;
        std::string name = node->children[ci]->tokenValue; ci++;
        ci++; 
        int constType = TYPE_UNKNOWN;
        if (ci < (int)node->children.size() && node->children[ci]->label == "<constant>") {
            constType = visitConstant(node->children[ci]); ci++;
        }
        ci++;
        addIdentifier(name, OBJ_CONST, constType, 0, 1, 0);
    }
    node->semType = TYPE_VOID;
}

int SemanticAnalyzer::visitConstant(ASTNode* node) {
    int type = TYPE_UNKNOWN;
    for (auto* c : node->children) {
        if (!c->isTerminal) continue;
        if (c->tokenType == "charcon") { type = TYPE_CHAR;    c->semType = type; break; }
        if (c->tokenType == "string")  { type = TYPE_STRING;  c->semType = type; break; }
        if (c->tokenType == "intcon")  { type = TYPE_INTEGER; c->semType = type; break; }
        if (c->tokenType == "realcon") { type = TYPE_REAL;    c->semType = type; break; }
        if (c->tokenType == "ident") {
            int idx = lookupIdent(c->tokenValue, false);
            if (idx >= 0) { type = tab[idx].type; c->semType = type; c->tabIdx = idx; }
            break;
        }
    }
    node->semType = type;
    return type;
}


void SemanticAnalyzer::visitTypeDeclaration(ASTNode* node) {
    int ci = 1;
    while (ci < (int)node->children.size()) {
        ASTNode* identNode = node->children[ci];
        if (!identNode->isTerminal || identNode->tokenType != "ident") break;
        std::string name = identNode->tokenValue; ci++;
        ci++; 
        int ref = 0, typeCode = TYPE_UNKNOWN;
        if (ci < (int)node->children.size() && node->children[ci]->label == "<type>") {
            typeCode = visitType(node->children[ci], ref); ci++;
        }
        ci++; 
        int idx = addIdentifier(name, OBJ_TYPE, typeCode, ref, 1, 0);
        identNode->tabIdx = idx;
        identNode->semType = typeCode;
        identNode->lexLevel = currentLevel;
    }
    node->semType = TYPE_VOID;
}

int SemanticAnalyzer::visitType(ASTNode* node, int& outRef) {
    outRef = 0;
    if (node->children.empty()) { node->semType = TYPE_UNKNOWN; return TYPE_UNKNOWN; }

    ASTNode* child = node->children[0];

    if (child->isTerminal && child->tokenType == "ident") {
        int idx = lookupIdent(child->tokenValue, false);
        if (idx < 0) {
            semanticError("Unknown type '" + child->tokenValue + "'");
            node->semType = TYPE_UNKNOWN;
            return TYPE_UNKNOWN;
        }
        if (tab[idx].obj != OBJ_TYPE) {
            semanticError("'" + child->tokenValue + "' is not a type");
            node->semType = TYPE_UNKNOWN;
            return TYPE_UNKNOWN;
        }
        child->tabIdx  = idx;
        child->semType = tab[idx].type;
        outRef         = tab[idx].ref;
        node->semType  = tab[idx].type;
        node->tabIdx   = idx;
        return tab[idx].type;
    }

    if (!child->isTerminal) {
        int t = TYPE_UNKNOWN;
        if      (child->label == "<array-type>")  t = visitArrayType(child, outRef);
        else if (child->label == "<range>")        t = visitRange(child, outRef);
        else if (child->label == "<enumerated>")   t = visitEnumerated(child, outRef);
        else if (child->label == "<record-type>")  t = visitRecordType(child, outRef);
        node->semType = t;
        return t;
    }

    node->semType = TYPE_UNKNOWN;
    return TYPE_UNKNOWN;
}

int SemanticAnalyzer::visitArrayType(ASTNode* node, int& outRef) {
    AtabEntry a;
    a.xtyp = TYPE_UNKNOWN;
    a.etyp = TYPE_UNKNOWN;
    a.eref = 0;
    a.low  = 0; a.high = 0;
    a.elsz = 1; a.size = 0;

    bool pastLbrack = false;
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "lbrack") { pastLbrack = true; continue; }
        if (c->isTerminal && c->tokenType == "rbrack") break;
        if (!pastLbrack) continue;

        if (!c->isTerminal && c->label == "<range>") {
            int rref = 0;
            a.xtyp = visitRange(c, rref, &a.low, &a.high);
        } else if (c->isTerminal && c->tokenType == "ident") {
            int idx = lookupIdent(c->tokenValue, false);
            if (idx >= 0) { a.xtyp = tab[idx].type; c->tabIdx = idx; }
        }
        break;
    }

    if (a.xtyp == TYPE_REAL)
        semanticError("Array index type cannot be Real");

    bool pastOfsy = false;
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ofsy") { pastOfsy = true; continue; }
        if (!pastOfsy) continue;
        if (!c->isTerminal && c->label == "<type>") {
            a.etyp = visitType(c, a.eref);
            a.elsz = getTypeSize(a.etyp, a.eref);
            if (a.high >= a.low)
                a.size = (a.high - a.low + 1) * a.elsz;
        }
        break;
    }

    atab.push_back(a);
    outRef        = (int)atab.size(); 
    node->semType = TYPE_ARRAY;
    return TYPE_ARRAY;
}

int SemanticAnalyzer::visitRange(ASTNode* node, int& outRef,int* low, int* high) {
    outRef = 0;
    if (node->children.size() < 4) { node->semType = TYPE_UNKNOWN; return TYPE_UNKNOWN; }

    int lowType  = visitConstant(node->children[0]);
    int highType = visitConstant(node->children[3]);

    if (lowType == TYPE_REAL || highType == TYPE_REAL)
        semanticError("Subrange bounds cannot be Real");
    if (lowType != highType && lowType != TYPE_UNKNOWN && highType != TYPE_UNKNOWN)
        semanticError("Subrange bounds must have the same type");

    auto extractInt = [&](ASTNode* constNode) -> int {
        bool neg = false;
        for (auto* c : constNode->children) {
            if (c->isTerminal && c->tokenType == "minus") neg = true;
            if (c->isTerminal && c->tokenType == "intcon") {
                int v = std::stoi(c->tokenValue);
                return neg ? -v : v;
            }
        }
        return 0;
    };

    int lo = extractInt(node->children[0]);
    int hi = extractInt(node->children[3]);

    if (low)  *low  = lo;
    if (high) *high = hi;

    RangeEntry re;
    re.low      = lo;
    re.high     = hi;
    re.baseType = (lowType != TYPE_UNKNOWN) ? lowType : TYPE_INTEGER;
    rtab.push_back(re);
    outRef = (int)rtab.size();  // 1-based

    node->semType = TYPE_SUBRANGE;
    return TYPE_SUBRANGE;
}

int SemanticAnalyzer::visitEnumerated(ASTNode* node, int& outRef) {
    outRef = 0;
    int pos = 0;
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ident") {
            c->semType = TYPE_ENUMERATED;
            addIdentifier(c->tokenValue, OBJ_CONST, TYPE_ENUMERATED, 0, 1, pos++);
        }
    }
    node->semType = TYPE_ENUMERATED;
    return TYPE_ENUMERATED;
}

int SemanticAnalyzer::visitRecordType(ASTNode* node, int& outRef) {
    int blockIdx = enterBlock();
    outRef = blockIdx;
    node->tabIdx = blockIdx;
    node->lexLevel = currentLevel;

    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<field-list>"){
            visitFieldList(c);
        }
    }

    leaveBlock();
    node->semType = TYPE_RECORD;
    return TYPE_RECORD;
}

void SemanticAnalyzer::visitFieldList(ASTNode* node) {
    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<field-part>")
            visitFieldPart(c);
    }
}

void SemanticAnalyzer::visitFieldPart(ASTNode* node) {
    ASTNode* identList = nullptr;
    ASTNode* typeNode  = nullptr;
    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<identifier-list>") identList = c;
        if (!c->isTerminal && c->label == "<type>") typeNode  = c;
    }
    if (!identList || !typeNode) return;

    int ref = 0;
    int typeCode = visitType(typeNode, ref);
    for (auto* c : identList->children) {
        if (c->isTerminal && c->tokenType == "ident") {
            int adr = btab[display[currentLevel]].vsze;
            btab[display[currentLevel]].vsze += getTypeSize(typeCode, ref);
            int idx = addIdentifier(c->tokenValue, OBJ_VAR, typeCode, ref, 1, adr);
            c->tabIdx = idx;
            c->semType = typeCode;
            c->lexLevel = currentLevel;
        }
    }
}


void SemanticAnalyzer::visitVarDeclaration(ASTNode* node) {
    int ci = 1;
    while (ci < (int)node->children.size()) {
        if (node->children[ci]->isTerminal &&
            node->children[ci]->tokenType != "ident" &&
            node->children[ci]->label != "<identifier-list>") break;

        ASTNode* identList = node->children[ci]; ci++;
        if (!identList || identList->label != "<identifier-list>") break;
        ci++; 
        if (ci >= (int)node->children.size()) break;
        ASTNode* typeNode = node->children[ci]; ci++;
        ci++;

        int ref = 0;
        int typeCode = visitType(typeNode, ref);

        for (auto* c : identList->children) {
            if (c->isTerminal && c->tokenType == "ident") {
                int adr = btab[display[currentLevel]].vsze;
                btab[display[currentLevel]].vsze += getTypeSize(typeCode, ref);
                int idx = addIdentifier(c->tokenValue, OBJ_VAR, typeCode, ref, 1, adr);
                c->tabIdx  = idx;
                c->semType = typeCode;
                c->lexLevel = currentLevel;
            }
        }
        typeNode->semType = typeCode;
    }
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitSubprogramDeclaration(ASTNode* node) {
    for (auto* c : node->children) {
        if (!c->isTerminal) {
            if      (c->label == "<procedure-declaration>") visitProcedureDeclaration(c);
            else if (c->label == "<function-declaration>")  visitFunctionDeclaration(c);
        }
    }
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitProcedureDeclaration(ASTNode* node) {
    std::string procName;
    ASTNode* paramList = nullptr;
    ASTNode* block     = nullptr;

    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ident" && procName.empty())
            procName = c->tokenValue;
        if (!c->isTerminal && c->label == "<formal-parameter-list>") paramList = c;
        if (!c->isTerminal && c->label == "<block>")                 block     = c;
    }

    int procIdx  = addIdentifier(procName, OBJ_PROC, TYPE_VOID, 0, 1, 0);
    int blockIdx = enterBlock();
    tab[procIdx].ref = blockIdx;

    if (paramList) visitFormalParameterList(paramList);
    btab[blockIdx].lpar = btab[display[currentLevel]].last;

    if (block) visitBlock(block);
    leaveBlock();
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitFunctionDeclaration(ASTNode* node) {
    std::string funcName;
    int returnType = TYPE_VOID;
    ASTNode* paramList = nullptr;
    ASTNode* block     = nullptr;
    bool seenColon = false;

    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "colon") { seenColon = true; continue; }
        if (c->isTerminal && c->tokenType == "ident") {
            if (funcName.empty())  funcName = c->tokenValue;
            else if (seenColon) {
                int idx = lookupIdent(c->tokenValue, false);
                if (idx >= 0 && tab[idx].obj == OBJ_TYPE) returnType = tab[idx].type;
            }
        }
        if (!c->isTerminal && c->label == "<formal-parameter-list>") paramList = c;
        if (!c->isTerminal && c->label == "<block>")                 block     = c;
    }

    int funcIdx  = addIdentifier(funcName, OBJ_FUNC, returnType, 0, 1, 0);
    int blockIdx = enterBlock();
    tab[funcIdx].ref = blockIdx;

    if (paramList) visitFormalParameterList(paramList);
    btab[blockIdx].lpar = btab[display[currentLevel]].last;

    if (block) visitBlock(block);
    leaveBlock();
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitFormalParameterList(ASTNode* node) {
    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<parameter-group>")
            visitParameterGroup(c);
    }
}

void SemanticAnalyzer::visitParameterGroup(ASTNode* node) {
    ASTNode* identList = nullptr;
    int typeCode = TYPE_UNKNOWN, ref = 0;

    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<identifier-list>") { identList = c; continue; }
        if (!c->isTerminal && c->label == "<array-type>") {
            typeCode = visitArrayType(c, ref); continue;
        }
        if (c->isTerminal && c->tokenType == "ident" && identList) {
            int idx = lookupIdent(c->tokenValue, false);
            if (idx >= 0 && tab[idx].obj == OBJ_TYPE) {
                typeCode = tab[idx].type;
                ref      = tab[idx].ref;
                c->tabIdx  = idx;
                c->semType = typeCode;
            }
        }
    }

    if (!identList) return;
    for (auto* c : identList->children) {
        if (c->isTerminal && c->tokenType == "ident") {
            int adr = btab[display[currentLevel]].psze;
            btab[display[currentLevel]].psze += getTypeSize(typeCode, ref);
            int idx = addIdentifier(c->tokenValue, OBJ_VAR, typeCode, ref, 1, adr);
            c->tabIdx   = idx;
            c->semType  = typeCode;
            c->lexLevel = currentLevel;
        }
    }
}

void SemanticAnalyzer::visitBlock(ASTNode* node) {
    ASTNode* compStmt = nullptr;
    for (auto* c : node->children) {
        if (!c->isTerminal) {
            if      (c->label == "<declaration-part>")   visitDeclarationPart(c);
            else if (c->label == "<compound-statement>") compStmt = c;
        }
    }
    if (compStmt) {
        compStmt->tabIdx   = display[currentLevel];
        compStmt->lexLevel = currentLevel;
        visitCompoundStatement(compStmt);
    }
    node->semType = TYPE_VOID;
}


void SemanticAnalyzer::visitCompoundStatement(ASTNode* node) {
    for (auto* c : node->children)
        if (!c->isTerminal && c->label == "<statement-list>")
            visitStatementList(c);
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitStatementList(ASTNode* node) {
    for (auto* c : node->children) {
        if (!c->isTerminal) visitStatement(c);
    }
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitStatement(ASTNode* node) {
    if (!node) return;
    const std::string& lbl = node->label;
    if      (lbl == "<assignment-statement>")   visitAssignmentStatement(node);
    else if (lbl == "<if-statement>")           visitIfStatement(node);
    else if (lbl == "<case-statement>")         visitCaseStatement(node);
    else if (lbl == "<while-statement>")        visitWhileStatement(node);
    else if (lbl == "<repeat-statement>")       visitRepeatStatement(node);
    else if (lbl == "<for-statement>")          visitForStatement(node);
    else if (lbl == "<procedure/function-call>")visitProcFuncCall(node);
    else if (lbl == "<compound-statement>")     visitCompoundStatement(node);
}

void SemanticAnalyzer::visitAssignmentStatement(ASTNode* node) {
    currentLine = getNodeLine(node);
    int targetType = TYPE_UNKNOWN;
    int targetRef  = 0;
    std::string targetName;
    ASTNode* exprNode = nullptr;
    bool seenBecomes = false;

    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "becomes") { seenBecomes = true; continue; }
        if (!seenBecomes) {
            if (c->isTerminal && c->tokenType == "ident") {
                int idx = lookupIdent(c->tokenValue);
                if (idx >= 0) {
                    targetType  = tab[idx].type;
                    targetRef   = tab[idx].ref;
                    targetName  = tab[idx].id;
                    c->semType  = targetType;
                    c->tabIdx   = idx;
                    c->lexLevel = tab[idx].lev;
                }
            } else if (!c->isTerminal && c->label == "<variable>") {
                targetType = visitVariable(c);
            }
        } else {
            if (!c->isTerminal && c->label == "<expression>") { exprNode = c; break; }
        }
    }

    int exprType = TYPE_UNKNOWN;
    if (exprNode) exprType = visitExpression(exprNode);

    if (!assignCompatible(targetType, exprType)) {
        semanticError("Type mismatch in assignment: cannot assign " +
                      typeCodeName(exprType) + " to " + typeCodeName(targetType));
    }

    if (targetType == TYPE_SUBRANGE && targetRef > 0 &&
        targetRef <= (int)rtab.size() && exprNode) {
        int constVal = 0;
        if (tryGetConstInt(exprNode, constVal)) {
            auto& range = rtab[targetRef - 1];
            if (constVal < range.low || constVal > range.high) {
                semanticError("Assignment-incompatible. Value " + std::to_string(constVal) +
                              " is out of range [" + std::to_string(range.low) + ".." +
                              std::to_string(range.high) + "] for variable '" + targetName + "'");
            }
        }
    }

    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitIfStatement(ASTNode* node) {
    currentLine = getNodeLine(node);
    ASTNode* cond = nullptr;
    bool seenThen = false;
    std::vector<ASTNode*> stmts;

    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "thensy") { seenThen = true; continue; }
        if (c->isTerminal) continue;
        if (c->label == "<expression>" && !seenThen) cond = c;
        else stmts.push_back(c);
    }

    if (cond) {
        int t = visitExpression(cond);
        if (t != TYPE_BOOLEAN && t != TYPE_UNKNOWN)
            semanticError("If condition must be Boolean, got " + typeCodeName(t));
    }
    for (auto* s : stmts) visitStatement(s);
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitCaseStatement(ASTNode* node) {
    ASTNode* expr  = nullptr;
    ASTNode* caseB = nullptr;
    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<expression>") expr  = c;
        if (!c->isTerminal && c->label == "<case-block>") caseB = c;
    }
    if (expr)  visitExpression(expr);
    if (caseB) visitCaseBlock(caseB);
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitCaseBlock(ASTNode* node) {
    for (auto* c : node->children) {
        if (c->isTerminal) continue;
        if      (c->label == "<constant>")   visitConstant(c);
        else if (c->label == "<case-block>") visitCaseBlock(c);
        else                                 visitStatement(c);
    }
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitWhileStatement(ASTNode* node) {
    currentLine = getNodeLine(node);
    ASTNode* cond = nullptr;
    ASTNode* body = nullptr;
    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<expression>")       cond = c;
        if (!c->isTerminal && c->label == "<compound-statement>") body = c;
    }

    if (cond) {
        int t = visitExpression(cond);
        if (t != TYPE_BOOLEAN && t != TYPE_UNKNOWN)
            semanticError("While condition must be Boolean, got " + typeCodeName(t));
    }
    if (body) {
        int blk = enterBlock();
        body->tabIdx   = blk;
        body->lexLevel = currentLevel;
        visitCompoundStatement(body);
        leaveBlock();
    }
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitRepeatStatement(ASTNode* node) {
    ASTNode* stmtList = nullptr;
    ASTNode* cond     = nullptr;
    bool seenUntil    = false;
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "untilsy") { seenUntil = true; continue; }
        if (c->isTerminal) continue;
        if (c->label == "<statement-list>") stmtList = c;
        if (c->label == "<expression>" && seenUntil) cond = c;
    }
    if (stmtList) visitStatementList(stmtList);
    if (cond) {
        int t = visitExpression(cond);
        if (t != TYPE_BOOLEAN && t != TYPE_UNKNOWN)
            semanticError("Repeat-until condition must be Boolean");
    }
    node->semType = TYPE_VOID;
}

void SemanticAnalyzer::visitForStatement(ASTNode* node) {
    currentLine = getNodeLine(node);
    std::string loopVar;
    std::vector<ASTNode*> exprs;
    ASTNode* body = nullptr;

    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ident") loopVar = c->tokenValue;
        if (!c->isTerminal && c->label == "<expression>") exprs.push_back(c);
        if (!c->isTerminal && c->label == "<compound-statement>") body = c;
    }

    if (!loopVar.empty()) {
        int idx = lookupIdent(loopVar);
        if (idx >= 0) {
            if (tab[idx].obj != OBJ_VAR)
                semanticError("For loop control must be a variable");
            else if (tab[idx].type != TYPE_INTEGER && tab[idx].type != TYPE_CHAR)
                semanticError("For loop control variable must be Integer or Char");
            for (auto* c : node->children) {
                if (c->isTerminal && c->tokenType == "ident") {
                    c->tabIdx   = idx;
                    c->semType  = tab[idx].type;
                    c->lexLevel = tab[idx].lev;
                    break;
                }
            }
        }
    }
    for (auto* e : exprs) visitExpression(e);
    if (body) {
        int blk = enterBlock();
        body->tabIdx   = blk;
        body->lexLevel = currentLevel;
        visitCompoundStatement(body);
        leaveBlock();
    }
    node->semType = TYPE_VOID;
}


std::vector<int> SemanticAnalyzer::getFormalParameterIndices(int calleeIdx) const {
    std::vector<int> params;
    if (calleeIdx < 0 || calleeIdx >= static_cast<int>(tab.size())) return params;

    int blockIdx = tab[calleeIdx].ref;
    if (blockIdx <= 0 || blockIdx >= static_cast<int>(btab.size())) return params;

    // lpar menunjuk parameter terakhir yang dideklarasikan. Chain-nya dibalik
    // dari urutan source, jadi nanti di-sort pakai adr supaya balik ke urutan asli
    int paramLevel = tab[calleeIdx].lev + 1;
    int i = btab[blockIdx].lpar;
    while (i > 0 && i < static_cast<int>(tab.size()) && tab[i].lev == paramLevel) {
        if (tab[i].obj == OBJ_VAR) params.push_back(i);
        i = tab[i].link;
    }

    std::sort(params.begin(), params.end(), [this](int a, int b) {
        return tab[a].adr < tab[b].adr;
    });
    return params;
}

std::vector<ASTNode*> SemanticAnalyzer::collectActualArguments(ASTNode* paramList) const {
    std::vector<ASTNode*> args;
    if (!paramList) return args;
    for (auto* c : paramList->children) {
        if (c && !c->isTerminal && c->label == "<expression>") args.push_back(c);
    }
    return args;
}

std::vector<ASTNode*> SemanticAnalyzer::collectIndexNodes(ASTNode* indexList) const {
    std::vector<ASTNode*> result;
    if (!indexList) return result;

    for (auto* c : indexList->children) {
        if (!c) continue;
        if (c->isTerminal && (c->tokenType == "intcon" || c->tokenType == "charcon" || c->tokenType == "ident")) {
            result.push_back(c);
        } else if (!c->isTerminal && c->label == "<index-list>") {
            auto nested = collectIndexNodes(c);
            result.insert(result.end(), nested.begin(), nested.end());
        }
    }
    return result;
}

int SemanticAnalyzer::inferIndexNodeType(ASTNode* node) {
    if (!node) return TYPE_UNKNOWN;

    if (node->isTerminal) {
        if (node->tokenType == "intcon") {
            node->semType = TYPE_INTEGER;
            return TYPE_INTEGER;
        }
        if (node->tokenType == "charcon") {
            node->semType = TYPE_CHAR;
            return TYPE_CHAR;
        }
        if (node->tokenType == "ident") {
            int idx = lookupIdent(node->tokenValue, false);
            if (idx < 0) {
                semanticError("Identifier '" + node->tokenValue + "' is not declared");
                node->semType = TYPE_UNKNOWN;
                return TYPE_UNKNOWN;
            }
            node->tabIdx = idx;
            node->lexLevel = tab[idx].lev;
            node->semType = tab[idx].type;
            return tab[idx].type;
        }
    }
    node->semType = TYPE_UNKNOWN;
    return TYPE_UNKNOWN;
}

void SemanticAnalyzer::visitProcFuncCall(ASTNode* node) {
    currentLine = getNodeLine(node);
    std::string funcName;
    ASTNode* paramList = nullptr;

    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ident") funcName = c->tokenValue;
        if (!c->isTerminal && c->label == "<parameter-list>") paramList = c;
    }

    std::vector<ASTNode*> args = collectActualArguments(paramList);
    std::vector<int> actualTypes;
    actualTypes.reserve(args.size());
    for (auto* arg : args) actualTypes.push_back(visitExpression(arg));
    if (paramList) paramList->semType = TYPE_VOID;

    const std::string lname = toLower(funcName);

    // Built-in I/O sengaja fleksibel: writeln/write boleh menerima berbagai tipe,
    // sedangkan readln minimal dicek targetnya berupa variabel oleh tahap lain/IC
    if (lname == "writeln" || lname == "write" || lname == "readln") {
        int idx = lookupIdent(funcName, false);
        if (idx >= 0) {
            node->tabIdx = idx;
            node->semType = TYPE_VOID;
            for (auto* c : node->children) {
                if (c->isTerminal && c->tokenType == "ident") {
                    c->tabIdx = idx;
                    c->semType = TYPE_VOID;
                }
            }
        } else if (lname == "write") {
            // write belum selalu didaftarkan sebagai predefined di versi awal,
            // tapi IC generator sudah mendukungnya sebagai built-in
            node->semType = TYPE_VOID;
        } else {
            semanticError("Undefined procedure/function: '" + funcName + "'");
            node->semType = TYPE_VOID;
        }
        return;
    }

    int idx = lookupIdent(funcName, false);
    if (idx < 0) {
        semanticError("Undefined procedure/function: '" + funcName + "'");
        node->semType = TYPE_VOID;
        return;
    }

    if (tab[idx].obj != OBJ_PROC && tab[idx].obj != OBJ_FUNC) {
        semanticError("'" + funcName + "' is not a procedure/function");
        node->semType = TYPE_UNKNOWN;
        return;
    }

    node->semType = (tab[idx].obj == OBJ_FUNC) ? tab[idx].type : TYPE_VOID;
    node->tabIdx  = idx;
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ident") {
            c->tabIdx  = idx;
            c->semType = node->semType;
        }
    }

    // Validasi jumlah dan tipe argumen
    std::vector<int> formals = getFormalParameterIndices(idx);
    if (actualTypes.size() != formals.size()) {
        semanticError("Argument count mismatch in call to '" + funcName + "': expected " +
                      std::to_string(formals.size()) + ", got " + std::to_string(actualTypes.size()));
        return;
    }

    for (std::size_t i = 0; i < formals.size(); ++i) {
        int formalType = tab[formals[i]].type;
        int actualType = actualTypes[i];
        if (!assignCompatible(formalType, actualType)) {
            semanticError("Argument " + std::to_string(i + 1) + " of '" + funcName +
                          "' expects " + typeCodeName(formalType) +
                          ", got " + typeCodeName(actualType));
        }
    }
}


int SemanticAnalyzer::visitExpression(ASTNode* node) {
    std::vector<ASTNode*> simpleExprs;
    ASTNode* relOp = nullptr;

    for (auto* c : node->children) {
        if (c->isTerminal && isRelOp(c->tokenType)) relOp = c;
        if (!c->isTerminal && c->label == "<simple-expression>") simpleExprs.push_back(c);
    }

    int type1 = TYPE_UNKNOWN;
    if (!simpleExprs.empty()) type1 = visitSimpleExpression(simpleExprs[0]);

    if (relOp && simpleExprs.size() >= 2) {
        int type2 = visitSimpleExpression(simpleExprs[1]);
        if (!typesCompatible(type1, type2) && type1 != TYPE_UNKNOWN && type2 != TYPE_UNKNOWN)
            semanticWarning("Potentially incompatible types in relational expression");
        node->semType = TYPE_BOOLEAN;
        return TYPE_BOOLEAN;
    }

    node->semType = type1;
    return type1;
}

int SemanticAnalyzer::visitSimpleExpression(ASTNode* node) {
    int ci = 0;
    auto& ch = node->children;
    int n = (int)ch.size();

    bool negated = false;
    if (ci < n && ch[ci]->isTerminal &&
        (ch[ci]->tokenType == "plus" || ch[ci]->tokenType == "minus")) {
        negated = (ch[ci]->tokenType == "minus");
        ch[ci]->semType = TYPE_VOID;
        ci++;
    }

    if (ci >= n) { node->semType = TYPE_UNKNOWN; return TYPE_UNKNOWN; }

    int resultType = ch[ci]->isTerminal ? TYPE_UNKNOWN : visitTerm(ch[ci]);
    ci++;

    if (negated && resultType != TYPE_INTEGER && resultType != TYPE_REAL
        && resultType != TYPE_UNKNOWN)
        semanticError("Unary minus requires a numeric operand");

    while (ci < n) {
        if (!ch[ci]->isTerminal) { resultType = visitTerm(ch[ci++]); continue; }
        const std::string& op = ch[ci]->tokenType;
        if (op != "plus" && op != "minus" && op != "orsy") break;
        ch[ci]->semType = TYPE_VOID; ci++;
        if (ci >= n) break;
        int termType = ch[ci]->isTerminal ? TYPE_UNKNOWN : visitTerm(ch[ci]);
        ci++;

        if (op == "orsy") {
            if ((resultType != TYPE_BOOLEAN && resultType != TYPE_UNKNOWN) ||
                (termType   != TYPE_BOOLEAN && termType   != TYPE_UNKNOWN))
                semanticError("'or' requires Boolean operands");
            resultType = TYPE_BOOLEAN;
        } else {
            if (isNumeric(resultType) && isNumeric(termType)) {
                resultType = (resultType == TYPE_REAL || termType == TYPE_REAL)
                             ? TYPE_REAL : TYPE_INTEGER;
            } else if (resultType != TYPE_UNKNOWN && termType != TYPE_UNKNOWN) {
                semanticError("Arithmetic operator requires numeric operands");
            }
        }
    }

    node->semType = resultType;
    return resultType;
}

int SemanticAnalyzer::visitTerm(ASTNode* node) {
    int ci = 0;
    auto& ch = node->children;
    int n = (int)ch.size();

    if (ci >= n) { node->semType = TYPE_UNKNOWN; return TYPE_UNKNOWN; }

    int resultType = ch[ci]->isTerminal ? TYPE_UNKNOWN : visitFactor(ch[ci]);
    ci++;

    while (ci < n) {
        if (!ch[ci]->isTerminal) { resultType = visitFactor(ch[ci++]); continue; }
        const std::string& op = ch[ci]->tokenType;
        if (op != "times" && op != "rdiv" && op != "idiv" &&
            op != "imod"  && op != "andsy") break;
        ch[ci]->semType = TYPE_VOID; ci++;
        if (ci >= n) break;
        int factType = ch[ci]->isTerminal ? TYPE_UNKNOWN : visitFactor(ch[ci]);
        ci++;

        if (op == "andsy") {
            if ((resultType != TYPE_BOOLEAN && resultType != TYPE_UNKNOWN) ||
                (factType   != TYPE_BOOLEAN && factType   != TYPE_UNKNOWN))
                semanticError("'and' requires Boolean operands");
            resultType = TYPE_BOOLEAN;
        } else if (op == "idiv" || op == "imod") {
            if ((resultType != TYPE_INTEGER && resultType != TYPE_UNKNOWN) ||
                (factType   != TYPE_INTEGER && factType   != TYPE_UNKNOWN))
                semanticError("'div'/'mod' requires Integer operands");
            resultType = TYPE_INTEGER;
        } else if (op == "rdiv") {
            if (!isNumeric(resultType) && resultType != TYPE_UNKNOWN)
                semanticError("'/' requires numeric operands");
            resultType = TYPE_REAL;
        } else { 
            if (isNumeric(resultType) && isNumeric(factType)) {
                resultType = (resultType == TYPE_REAL || factType == TYPE_REAL)
                             ? TYPE_REAL : TYPE_INTEGER;
            } else if (resultType != TYPE_UNKNOWN && factType != TYPE_UNKNOWN) {
                semanticError("'*' requires numeric operands");
            }
        }
    }

    node->semType = resultType;
    return resultType;
}

int SemanticAnalyzer::visitFactor(ASTNode* node) {
    if (node->children.empty()) { node->semType = TYPE_UNKNOWN; return TYPE_UNKNOWN; }

    ASTNode* first = node->children[0];
    int t = TYPE_UNKNOWN;

    if (first->isTerminal) {
        if      (first->tokenType == "intcon")  t = TYPE_INTEGER;
        else if (first->tokenType == "realcon") t = TYPE_REAL;
        else if (first->tokenType == "charcon") t = TYPE_CHAR;
        else if (first->tokenType == "string")  t = TYPE_STRING;
        else if (first->tokenType == "notsy") {
            if (node->children.size() >= 2) {
                t = visitFactor(node->children[1]);
                if (t != TYPE_BOOLEAN && t != TYPE_UNKNOWN)
                    semanticError("'not' requires Boolean operand");
                t = TYPE_BOOLEAN;
            }
        } else if (first->tokenType == "lparent") {
            for (auto* c : node->children)
                if (!c->isTerminal && c->label == "<expression>")
                    t = visitExpression(c);
        } else if (first->tokenType == "ident") {
            int idx = lookupIdent(first->tokenValue);
            if (idx >= 0) {
                t = tab[idx].type;
                first->tabIdx   = idx;
                first->lexLevel = tab[idx].lev;
            }
        }
        first->semType = t;
    } else {
        if (first->label == "<procedure/function-call>") {
            visitProcFuncCall(first);
            t = first->semType;
        } else if (first->label == "<variable>") {
            t = visitVariable(first);
        } else {
            t = TYPE_UNKNOWN;
        }
    }

    node->semType = t;
    return t;
}

int SemanticAnalyzer::visitVariable(ASTNode* node) {
    ASTNode* identNode = nullptr;
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ident") { identNode = c; break; }
    }
    if (!identNode) { node->semType = TYPE_UNKNOWN; return TYPE_UNKNOWN; }

    int idx = lookupIdent(identNode->tokenValue);
    if (idx < 0) { node->semType = TYPE_UNKNOWN; return TYPE_UNKNOWN; }

    identNode->tabIdx   = idx;
    identNode->semType  = tab[idx].type;
    identNode->lexLevel = tab[idx].lev;

    int curType = tab[idx].type;
    int curRef  = tab[idx].ref;

    for (auto* c : node->children) {
        if (!c->isTerminal && c->label == "<component-variable>") {
            int newRef = 0;
            curType = visitComponentVariable(c, curType, curRef, newRef);
            curRef  = newRef;
        }
    }

    node->semType = curType;
    node->tabIdx  = idx;
    node->lexLevel = tab[idx].lev;
    return curType;
}

int SemanticAnalyzer::visitComponentVariable(ASTNode* node,
                                              int parentType, int parentRef, int& newRef) {
    newRef = 0;
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "lbrack") {
            ASTNode* indexList = nullptr;
            for (auto* sub : node->children) {
                if (!sub->isTerminal && sub->label == "<index-list>") {
                    indexList = sub;
                    break;
                }
            }

            if (parentType == TYPE_ARRAY && parentRef > 0 &&
                parentRef <= (int)atab.size()) {
                auto& arr = atab[parentRef - 1];
                auto indexNodes = collectIndexNodes(indexList);

                // Saat array memakai subrange 1..3, index aktual harus integer
                // Kalau char dipakai, jangan tunggu sampai runtime jadi ASCII 120
                if (indexNodes.empty()) {
                    semanticError("Array index is missing");
                }
                for (auto* idxNode : indexNodes) {
                    int actualType = inferIndexNodeType(idxNode);
                    bool ok = false;
                    if (arr.xtyp == TYPE_SUBRANGE) {
                        ok = (actualType == TYPE_INTEGER || actualType == TYPE_SUBRANGE || actualType == TYPE_UNKNOWN);
                    } else {
                        ok = typesCompatible(arr.xtyp, actualType) || actualType == TYPE_UNKNOWN;
                    }
                    if (!ok) {
                        semanticError("Array index type mismatch: expected " +
                                      typeCodeName(arr.xtyp == TYPE_SUBRANGE ? TYPE_INTEGER : arr.xtyp) +
                                      ", got " + typeCodeName(actualType));
                    }
                }

                newRef = arr.eref;
                return arr.etyp;
            }
            if (parentType != TYPE_ARRAY && parentType != TYPE_UNKNOWN)
                semanticError("Cannot use index notation on non-array type '" +
                              typeCodeName(parentType) + "'");
            return TYPE_UNKNOWN;
        }
        if (c->isTerminal && c->tokenType == "period") {
            if (parentType == TYPE_RECORD && parentRef > 0 &&
                parentRef < (int)btab.size()) {
                ASTNode* fieldNode = nullptr;
                for (auto* sub : node->children)
                    if (sub->isTerminal && sub->tokenType == "ident") { fieldNode = sub; break; }

                if (fieldNode) {
                    int i = btab[parentRef].last;
                    while (i > 0) {
                        if (toLower(tab[i].id) == toLower(fieldNode->tokenValue)) {
                            fieldNode->tabIdx  = i;
                            fieldNode->semType = tab[i].type;
                            newRef = tab[i].ref;
                            return tab[i].type;
                        }
                        i = tab[i].link;
                    }
                    semanticError("Undefined record field: " + fieldNode->tokenValue);
                }
            } else if (parentType != TYPE_RECORD && parentType != TYPE_UNKNOWN) {
                semanticError("Cannot use field access on non-record type '" +
                              typeCodeName(parentType) + "'");
            }
            return TYPE_UNKNOWN;
        }
    }
    return parentType;
}

void SemanticAnalyzer::visitIndexList(ASTNode* node) {
    for (auto* c : node->children) {
        if (c->isTerminal && c->tokenType == "ident") {
            int idx = lookupIdent(c->tokenValue, false);
            if (idx >= 0) { c->tabIdx = idx; c->semType = tab[idx].type; }
        }
        if (!c->isTerminal && c->label == "<index-list>") visitIndexList(c);
    }
}


// printAnnotatedTree and printSymbolTables have been moved to ASTDecoratedPrinter.
// SemanticAnalyzer::analyze() uses ASTDecoratedPrinter::printAll() instead.
