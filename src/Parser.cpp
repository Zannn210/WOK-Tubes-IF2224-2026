#include "Parser.hpp"
#include <iostream>
#include <cstdlib>

// ── Token file loader ───────────────────────────────────────────────────────

std::vector<Token> loadTokensFromFile(const std::string& filename) {
    std::vector<Token> tokens;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        Token t;
        size_t pos = line.find(" (");
        if (pos != std::string::npos) {
            t.type  = line.substr(0, pos);
            t.value = line.substr(pos + 2, line.length() - pos - 3);
        } else {
            t.type  = line;
            t.value = "";
        }
        // Skip comment tokens — they are not part of the parse tree
        if (t.type == "comment") continue;
        tokens.push_back(t);
    }
    return tokens;
}

// ── Constructor / Destructor ────────────────────────────────────────────────

Parser::Parser(const std::vector<Token>& t, const std::string& outFilename)
    : tokens(t), currentIndex(0), indentLevel(0) {
    outFile.open(outFilename);
}

Parser::~Parser() {
    if (outFile.is_open()) outFile.close();
}

// ── Utility helpers ─────────────────────────────────────────────────────────

Token Parser::currentToken() {
    if (currentIndex < (int)tokens.size()) return tokens[currentIndex];
    return {"EOF", ""};
}

Token Parser::lookahead(int offset) {
    int idx = currentIndex + offset;
    if (idx < (int)tokens.size()) return tokens[idx];
    return {"EOF", ""};
}

void Parser::advance() {
    if (currentIndex < (int)tokens.size()) currentIndex++;
}

bool Parser::check(const std::string& type) {
    return currentToken().type == type;
}

void Parser::printNode(const std::string& nodeName) {
    std::string indent;
    for (int i = 0; i < indentLevel; i++) indent += "|   ";
    if (indentLevel > 0) indent += "|-- ";

    std::cout << indent << nodeName << "\n";
    if (outFile.is_open()) outFile << indent << nodeName << "\n";
}

bool Parser::isRelationalOp() {
    const std::string& t = currentToken().type;
    return t == "eql" || t == "neq" || t == "gtr" ||
           t == "geq" || t == "lss" || t == "leq";
}

bool Parser::isAdditiveOp() {
    const std::string& t = currentToken().type;
    return t == "plus" || t == "minus" || t == "orsy";
}

bool Parser::isMultiplicativeOp() {
    const std::string& t = currentToken().type;
    return t == "times" || t == "rdiv" || t == "idiv" || t == "imod" || t == "andsy";
}

// ── match — consumes a token and returns a terminal ASTNode ─────────────────

ASTNode* Parser::match(const std::string& expectedType) {
    Token t = currentToken();
    if (t.type == expectedType) {
        std::string label = t.type;
        if (!t.value.empty()) label += "(" + t.value + ")";

        printNode(label);
        advance();
        return new ASTNode(label, true, t.type, t.value);
    }
    std::cerr << "\n[SYNTAX ERROR] Unexpected token '" << t.type
              << "', expected '" << expectedType << "'\n";
    exit(1);
}

// ── Entry point ─────────────────────────────────────────────────────────────

ASTNode* Parser::parse() {
    ASTNode* root = parseProgram();
    if (!check("EOF")) {
        std::cerr << "\n[SYNTAX ERROR] Unexpected token '"
                  << currentToken().type << "' after program end\n";
        exit(1);
    }
    return root;
}

// ── Program structure ───────────────────────────────────────────────────────

ASTNode* Parser::parseProgram() {
    auto* node = new ASTNode("<program>");
    printNode("<program>");
    indentLevel++;

    if (check("programsy"))
        node->addChild(parseProgramHeader());

    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(match("period"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseProgramHeader() {
    auto* node = new ASTNode("<program-header>");
    printNode("<program-header>");
    indentLevel++;

    node->addChild(match("programsy"));
    node->addChild(match("ident"));
    node->addChild(match("semicolon"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseDeclarationPart() {
    auto* node = new ASTNode("<declaration-part>");
    printNode("<declaration-part>");
    indentLevel++;

    while (check("constsy"))
        node->addChild(parseConstDeclaration());
    while (check("typesy"))
        node->addChild(parseTypeDeclaration());
    while (check("varsy"))
        node->addChild(parseVarDeclaration());
    while (check("proceduresy") || check("functionsy"))
        node->addChild(parseSubprogramDeclaration());

    indentLevel--;
    return node;
}

// ── Declarations ────────────────────────────────────────────────────────────

ASTNode* Parser::parseConstDeclaration() {
    auto* node = new ASTNode("<const-declaration>");
    printNode("<const-declaration>");
    indentLevel++;

    node->addChild(match("constsy"));
    do {
        node->addChild(match("ident"));
        node->addChild(match("eql"));
        node->addChild(parseConstant());
        node->addChild(match("semicolon"));
    } while (check("ident"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseConstant() {
    auto* node = new ASTNode("<constant>");
    printNode("<constant>");
    indentLevel++;

    if (check("charcon")) {
        node->addChild(match("charcon"));
    } else if (check("string")) {
        node->addChild(match("string"));
    } else {
        if (check("plus"))       node->addChild(match("plus"));
        else if (check("minus")) node->addChild(match("minus"));

        if      (check("ident"))   node->addChild(match("ident"));
        else if (check("intcon"))  node->addChild(match("intcon"));
        else if (check("realcon")) node->addChild(match("realcon"));
        else {
            std::cerr << "[SYNTAX ERROR] Expected constant, got '"
                      << currentToken().type << "'\n";
            exit(1);
        }
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseTypeDeclaration() {
    auto* node = new ASTNode("<type-declaration>");
    printNode("<type-declaration>");
    indentLevel++;

    node->addChild(match("typesy"));
    do {
        node->addChild(match("ident"));
        node->addChild(match("eql"));
        node->addChild(parseType());
        node->addChild(match("semicolon"));
    } while (check("ident"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseType() {
    auto* node = new ASTNode("<type>");
    printNode("<type>");
    indentLevel++;

    if (check("arraysy")) {
        node->addChild(parseArrayType());
    } else if (check("lparent")) {
        node->addChild(parseEnumerated());
    } else if (check("recordsy")) {
        node->addChild(parseRecordType());
    } else if (check("intcon")  || check("realcon") || check("charcon") ||
               check("string")  || check("plus")    || check("minus")) {
        node->addChild(parseRange());
    } else if (check("ident")) {
        // Disambiguate between plain type name and subrange 'ident..ident'
        if (lookahead(1).type == "period" && lookahead(2).type == "period") {
            node->addChild(parseRange());
        } else {
            node->addChild(match("ident"));
        }
    } else {
        std::cerr << "[SYNTAX ERROR] Expected type, got '"
                  << currentToken().type << "'\n";
        exit(1);
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseArrayType() {
    auto* node = new ASTNode("<array-type>");
    printNode("<array-type>");
    indentLevel++;

    node->addChild(match("arraysy"));
    node->addChild(match("lbrack"));

    // Index type: either a range or a named type identifier
    if (check("ident") && lookahead(1).type != "period") {
        node->addChild(match("ident"));
    } else {
        node->addChild(parseRange());
    }

    node->addChild(match("rbrack"));
    node->addChild(match("ofsy"));
    node->addChild(parseType());

    indentLevel--;
    return node;
}

ASTNode* Parser::parseRange() {
    auto* node = new ASTNode("<range>");
    printNode("<range>");
    indentLevel++;

    node->addChild(parseConstant());
    node->addChild(match("period"));
    node->addChild(match("period"));
    node->addChild(parseConstant());

    indentLevel--;
    return node;
}

ASTNode* Parser::parseEnumerated() {
    auto* node = new ASTNode("<enumerated>");
    printNode("<enumerated>");
    indentLevel++;

    node->addChild(match("lparent"));
    node->addChild(match("ident"));
    while (check("comma")) {
        node->addChild(match("comma"));
        node->addChild(match("ident"));
    }
    node->addChild(match("rparent"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseRecordType() {
    auto* node = new ASTNode("<record-type>");
    printNode("<record-type>");
    indentLevel++;

    node->addChild(match("recordsy"));
    node->addChild(parseFieldList());
    node->addChild(match("endsy"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseFieldList() {
    auto* node = new ASTNode("<field-list>");
    printNode("<field-list>");
    indentLevel++;

    node->addChild(parseFieldPart());
    while (check("semicolon")) {
        node->addChild(match("semicolon"));
        if (check("endsy")) break;
        node->addChild(parseFieldPart());
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseFieldPart() {
    auto* node = new ASTNode("<field-part>");
    printNode("<field-part>");
    indentLevel++;

    node->addChild(parseIdentifierList());
    node->addChild(match("colon"));
    node->addChild(parseType());

    indentLevel--;
    return node;
}

ASTNode* Parser::parseVarDeclaration() {
    auto* node = new ASTNode("<var-declaration>");
    printNode("<var-declaration>");
    indentLevel++;

    node->addChild(match("varsy"));
    do {
        node->addChild(parseIdentifierList());
        node->addChild(match("colon"));
        node->addChild(parseType());
        node->addChild(match("semicolon"));
    } while (check("ident"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseIdentifierList() {
    auto* node = new ASTNode("<identifier-list>");
    printNode("<identifier-list>");
    indentLevel++;

    node->addChild(match("ident"));
    while (check("comma")) {
        node->addChild(match("comma"));
        node->addChild(match("ident"));
    }

    indentLevel--;
    return node;
}

// ── Subprograms ─────────────────────────────────────────────────────────────

ASTNode* Parser::parseSubprogramDeclaration() {
    auto* node = new ASTNode("<subprogram-declaration>");
    printNode("<subprogram-declaration>");
    indentLevel++;

    if (check("proceduresy")) node->addChild(parseProcedureDeclaration());
    else                      node->addChild(parseFunctionDeclaration());

    indentLevel--;
    return node;
}

ASTNode* Parser::parseProcedureDeclaration() {
    auto* node = new ASTNode("<procedure-declaration>");
    printNode("<procedure-declaration>");
    indentLevel++;

    node->addChild(match("proceduresy"));
    node->addChild(match("ident"));
    if (check("lparent")) node->addChild(parseFormalParameterList());
    node->addChild(match("semicolon"));
    node->addChild(parseBlock());
    node->addChild(match("semicolon"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseFunctionDeclaration() {
    auto* node = new ASTNode("<function-declaration>");
    printNode("<function-declaration>");
    indentLevel++;

    node->addChild(match("functionsy"));
    node->addChild(match("ident"));
    if (check("lparent")) node->addChild(parseFormalParameterList());
    node->addChild(match("colon"));
    node->addChild(match("ident")); // return type
    node->addChild(match("semicolon"));
    node->addChild(parseBlock());
    node->addChild(match("semicolon"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseBlock() {
    auto* node = new ASTNode("<block>");
    printNode("<block>");
    indentLevel++;

    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());

    indentLevel--;
    return node;
}

ASTNode* Parser::parseFormalParameterList() {
    auto* node = new ASTNode("<formal-parameter-list>");
    printNode("<formal-parameter-list>");
    indentLevel++;

    node->addChild(match("lparent"));
    node->addChild(parseParameterGroup());
    while (check("semicolon")) {
        node->addChild(match("semicolon"));
        node->addChild(parseParameterGroup());
    }
    node->addChild(match("rparent"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseParameterGroup() {
    auto* node = new ASTNode("<parameter-group>");
    printNode("<parameter-group>");
    indentLevel++;

    node->addChild(parseIdentifierList());
    node->addChild(match("colon"));
    if (check("arraysy")) node->addChild(parseArrayType());
    else                  node->addChild(match("ident"));

    indentLevel--;
    return node;
}

// ── Compound statement and statement list ───────────────────────────────────

ASTNode* Parser::parseCompoundStatement() {
    auto* node = new ASTNode("<compound-statement>");
    printNode("<compound-statement>");
    indentLevel++;

    node->addChild(match("beginsy"));
    node->addChild(parseStatementList());
    node->addChild(match("endsy"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseStatementList() {
    auto* node = new ASTNode("<statement-list>");
    printNode("<statement-list>");
    indentLevel++;

    // The first statement may be absent (empty statement is valid in M3)
    if (!check("endsy") && !check("untilsy")) {
        ASTNode* s = parseStatement();
        if (s) node->addChild(s);
    }

    while (check("semicolon")) {
        node->addChild(match("semicolon"));
        if (check("endsy") || check("untilsy")) break;
        ASTNode* s = parseStatement();
        if (s) node->addChild(s);
    }

    indentLevel--;
    return node;
}

// parseStatement: dispatches to the appropriate rule.
// Returns nullptr for an empty statement (no node printed).
ASTNode* Parser::parseStatement() {
    const std::string t = currentToken().type;

    if (t == "ident") {
        const std::string next = lookahead(1).type;
        if (next == "lparent") {
            return parseProcFuncCall();
        } else if (next == "becomes" || next == "lbrack" || next == "period") {
            return parseAssignmentStatement();
        } else {
            // Bare identifier followed by end-of-statement marker: empty statement
            if (next == "semicolon" || next == "endsy" ||
                next == "elsesy"    || next == "untilsy") {
                return nullptr;
            }
            return parseAssignmentStatement();
        }
    } else if (t == "ifsy")       return parseIfStatement();
    else if (t == "casesy")       return parseCaseStatement();
    else if (t == "whilesy")      return parseWhileStatement();
    else if (t == "repeatsy")     return parseRepeatStatement();
    else if (t == "forsy")        return parseForStatement();
    else if (t == "beginsy")      return parseCompoundStatement();

    // Empty statement
    return nullptr;
}

// ── Statements ──────────────────────────────────────────────────────────────

ASTNode* Parser::parseVariable() {
    auto* node = new ASTNode("<variable>");
    printNode("<variable>");
    indentLevel++;

    node->addChild(match("ident"));
    while (check("lbrack") || check("period"))
        node->addChild(parseComponentVariable());

    indentLevel--;
    return node;
}

ASTNode* Parser::parseComponentVariable() {
    auto* node = new ASTNode("<component-variable>");
    printNode("<component-variable>");
    indentLevel++;

    if (check("lbrack")) {
        node->addChild(match("lbrack"));
        node->addChild(parseIndexList());
        node->addChild(match("rbrack"));
    } else {
        node->addChild(match("period"));
        node->addChild(match("ident"));
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseIndexList() {
    auto* node = new ASTNode("<index-list>");
    printNode("<index-list>");
    indentLevel++;

    if      (check("intcon"))  node->addChild(match("intcon"));
    else if (check("charcon")) node->addChild(match("charcon"));
    else if (check("ident"))   node->addChild(match("ident"));
    else {
        std::cerr << "[SYNTAX ERROR] Expected index, got '"
                  << currentToken().type << "'\n";
        exit(1);
    }

    while (check("comma")) {
        node->addChild(match("comma"));
        node->addChild(parseIndexList()); // recursive for multi-dim
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseAssignmentStatement() {
    auto* node = new ASTNode("<assignment-statement>");
    printNode("<assignment-statement>");
    indentLevel++;

    // Simple 'ident :=' or complex 'variable :='
    bool simpleLhs = check("ident") && lookahead(1).type == "becomes";
    if (simpleLhs) node->addChild(match("ident"));
    else           node->addChild(parseVariable());

    node->addChild(match("becomes"));
    node->addChild(parseExpression());

    indentLevel--;
    return node;
}

ASTNode* Parser::parseIfStatement() {
    auto* node = new ASTNode("<if-statement>");
    printNode("<if-statement>");
    indentLevel++;

    node->addChild(match("ifsy"));
    node->addChild(parseExpression());
    node->addChild(match("thensy"));
    node->addChild(parseStatement());
    if (check("elsesy")) {
        node->addChild(match("elsesy"));
        node->addChild(parseStatement());
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseCaseStatement() {
    auto* node = new ASTNode("<case-statement>");
    printNode("<case-statement>");
    indentLevel++;

    node->addChild(match("casesy"));
    node->addChild(parseExpression());
    node->addChild(match("ofsy"));
    node->addChild(parseCaseBlock());
    node->addChild(match("endsy"));

    indentLevel--;
    return node;
}

ASTNode* Parser::parseCaseBlock() {
    auto* node = new ASTNode("<case-block>");
    printNode("<case-block>");
    indentLevel++;

    node->addChild(parseConstant());
    while (check("comma")) {
        node->addChild(match("comma"));
        node->addChild(parseConstant());
    }
    node->addChild(match("colon"));
    ASTNode* s = parseStatement();
    if (s) node->addChild(s);

    while (check("semicolon")) {
        node->addChild(match("semicolon"));
        if (check("endsy")) break;
        node->addChild(parseCaseBlock());
    }

    indentLevel--;
    return node;
}

// M3 revision: body must be compound-statement
ASTNode* Parser::parseWhileStatement() {
    auto* node = new ASTNode("<while-statement>");
    printNode("<while-statement>");
    indentLevel++;

    node->addChild(match("whilesy"));
    node->addChild(parseExpression());
    node->addChild(match("dosy"));
    node->addChild(parseCompoundStatement()); // M3: compound-statement required

    indentLevel--;
    return node;
}

ASTNode* Parser::parseRepeatStatement() {
    auto* node = new ASTNode("<repeat-statement>");
    printNode("<repeat-statement>");
    indentLevel++;

    node->addChild(match("repeatsy"));
    node->addChild(parseStatementList());
    node->addChild(match("untilsy"));
    node->addChild(parseExpression());

    indentLevel--;
    return node;
}

// M3 revision: body must be compound-statement
ASTNode* Parser::parseForStatement() {
    auto* node = new ASTNode("<for-statement>");
    printNode("<for-statement>");
    indentLevel++;

    node->addChild(match("forsy"));
    node->addChild(match("ident"));
    node->addChild(match("becomes"));
    node->addChild(parseExpression());

    if      (check("tosy"))      node->addChild(match("tosy"));
    else if (check("downtosy"))  node->addChild(match("downtosy"));
    else {
        std::cerr << "[SYNTAX ERROR] Expected 'to' or 'downto', got '"
                  << currentToken().type << "'\n";
        exit(1);
    }

    node->addChild(parseExpression());
    node->addChild(match("dosy"));
    node->addChild(parseCompoundStatement()); // M3: compound-statement required

    indentLevel--;
    return node;
}

ASTNode* Parser::parseProcFuncCall() {
    auto* node = new ASTNode("<procedure/function-call>");
    printNode("<procedure/function-call>");
    indentLevel++;

    node->addChild(match("ident"));
    if (check("lparent")) {
        node->addChild(match("lparent"));
        if (!check("rparent")) node->addChild(parseParameterList());
        node->addChild(match("rparent"));
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseParameterList() {
    auto* node = new ASTNode("<parameter-list>");
    printNode("<parameter-list>");
    indentLevel++;

    node->addChild(parseExpression());
    while (check("comma")) {
        node->addChild(match("comma"));
        node->addChild(parseExpression());
    }

    indentLevel--;
    return node;
}

// ── Expressions ─────────────────────────────────────────────────────────────

ASTNode* Parser::parseExpression() {
    auto* node = new ASTNode("<expression>");
    printNode("<expression>");
    indentLevel++;

    node->addChild(parseSimpleExpression());
    if (isRelationalOp()) {
        node->addChild(match(currentToken().type));
        node->addChild(parseSimpleExpression());
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseSimpleExpression() {
    auto* node = new ASTNode("<simple-expression>");
    printNode("<simple-expression>");
    indentLevel++;

    if      (check("plus"))  node->addChild(match("plus"));
    else if (check("minus")) node->addChild(match("minus"));

    node->addChild(parseTerm());
    while (isAdditiveOp()) {
        node->addChild(match(currentToken().type));
        node->addChild(parseTerm());
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseTerm() {
    auto* node = new ASTNode("<term>");
    printNode("<term>");
    indentLevel++;

    node->addChild(parseFactor());
    while (isMultiplicativeOp()) {
        node->addChild(match(currentToken().type));
        node->addChild(parseFactor());
    }

    indentLevel--;
    return node;
}

ASTNode* Parser::parseFactor() {
    auto* node = new ASTNode("<factor>");
    printNode("<factor>");
    indentLevel++;

    if (check("intcon")) {
        node->addChild(match("intcon"));
    } else if (check("realcon")) {
        node->addChild(match("realcon"));
    } else if (check("charcon")) {
        node->addChild(match("charcon"));
    } else if (check("string")) {
        node->addChild(match("string"));
    } else if (check("lparent")) {
        node->addChild(match("lparent"));
        node->addChild(parseExpression());
        node->addChild(match("rparent"));
    } else if (check("notsy")) {
        node->addChild(match("notsy"));
        node->addChild(parseFactor());
    } else if (check("ident")) {
        if (lookahead(1).type == "lparent") {
            node->addChild(parseProcFuncCall());
        } else if (lookahead(1).type == "lbrack" || lookahead(1).type == "period") {
            node->addChild(parseVariable());
        } else {
            node->addChild(match("ident"));
        }
    } else {
        std::cerr << "[SYNTAX ERROR] Unexpected token in factor: '"
                  << currentToken().type << "'\n";
        exit(1);
    }

    indentLevel--;
    return node;
}
