#include "Parser.hpp"
#include <iostream>
#include <cstdlib>

//utility dan file loader

std::vector<Token> loadTokensFromFile(const std::string& filename) {
    std::vector<Token> tokens;
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        Token t;
        size_t pos = line.find(" (");
        if (pos != std::string::npos) {
            t.type = line.substr(0, pos);
            t.value = line.substr(pos + 2, line.length() - pos - 3);
        } else {
            t.type = line;
            t.value = "";
        }
        if (t.type == "comment") continue;
        tokens.push_back(t);
    }
    return tokens;
}

Parser::Parser(const std::vector<Token>& t, const std::string& outFilename) 
    : tokens(t), currentIndex(0), indentLevel(0) {
    outFile.open(outFilename);
}

Parser::~Parser() {
    if (outFile.is_open()) outFile.close();
}

Token Parser::currentToken() {
    if (currentIndex < (int)tokens.size()) return tokens[currentIndex];
    return {"EOF", ""}; 
}

Token Parser::lookahead(int offset) {
    if (currentIndex + offset < (int)tokens.size()) return tokens[currentIndex + offset];
    return {"EOF", ""};
}

void Parser::advance() {
    if (currentIndex < (int)tokens.size()) currentIndex++;
}

bool Parser::check(const std::string& type){
    return currentToken().type == type;
}


void Parser::printNode(const std::string& nodeName) {
    std::string indent = "";
    for(int i = 0; i < indentLevel; i++) indent += "|   ";
    if (indentLevel > 0) indent += "|-- ";

    std::cout << indent << nodeName << "\n";
    if (outFile.is_open()) {
        outFile << indent << nodeName << "\n";
    }
}

bool Parser::isRelationalOp(){
    std::string t = currentToken().type;
    return t == "eql" || t == "neq" || t == "gtr" || t == "geq" || t == "lss" || t == "leq";
}

bool Parser::isAdditiveOp(){
    std::string t = currentToken().type;
    return t == "plus" || t == "minus" || t == "orsy";
}
bool Parser::isMultiplecativeOp(){
    std::string t = currentToken().type;
    return t == "times" || t == "rdiv" || t == "idiv" || t == "imod" || t == "andsy";
}
bool Parser::match(const std::string& expectedType) {
    Token t = currentToken();
    if (t.type == expectedType) {
        std::string printText = t.type;
        if (!t.value.empty()) printText += " (" + t.value + ")";
        
        printNode(printText);
        advance();
        return true;
    } else {
        std::cerr << "\n[SYNTAX ERROR] Unexpected token '" << t.type << "', expected '" << expectedType << "'\n";
        exit(1); 
    }
}

void Parser::parse() {
    parseProgram();
    if (!check("EOF")) {
        std::cerr << "\n[SYNTAX ERROR] Unexpected token '" << currentToken().type
                  << "' after program end\n";
        exit(1);
    }
}

//root program

void Parser::parseProgram() {
    printNode("<program>");
    indentLevel++;
    
    if (check("programsy")) {
        parseProgramHeader();
    }
    parseDeclarationPart(); 
    parseCompoundStatement(); 
    match("period");
    
    indentLevel--;
}

void Parser::parseProgramHeader() {
    printNode("<program-header>");
    indentLevel++;
    
    match("programsy");
    match("ident");
    match("semicolon");
    
    indentLevel--;
}

void Parser::parseDeclarationPart(){
    printNode("<declaration-part>");
    indentLevel++;
    while (check("constsy"))
    {
        parseConstDeclaration();
    }
    while (check("typesy"))
    {
        parseTypeDeclaration();
    }
    while (check("varsy"))
    {
        parseVarDeclaration();
    }
    while (check("proceduresy") || check("functionsy"))
    {
        parseSubprogramDeclaration();
    }
    indentLevel--;
    
}

void Parser::parseConstDeclaration() {
    printNode("<const-declaration>");
    indentLevel++;
    match("constsy");
    do {
        match("ident");
        match("eql");
        parseConstant();
        match("semicolon");
    } while (check("ident"));
    indentLevel--;
}

void Parser::parseConstant() {
    printNode("<constant>");
    indentLevel++;
    if (check("charcon")) {
        match("charcon");
    } else if (check("string")) {
        match("string");
    } else {
        if (check("plus"))  match("plus");
        else if (check("minus")) match("minus");

        if (check("ident"))       match("ident");
        else if (check("intcon")) match("intcon");
        else if (check("realcon")) match("realcon");
        else {
            std::cerr << "[SYNTAX ERROR] Expected constant, got '"
                      << currentToken().type << "'\n";
            exit(1);
        }
    }
    indentLevel--;
}

void Parser::parseTypeDeclaration() {
    printNode("<type-declaration>");
    indentLevel++;
    match("typesy");
    do {
        match("ident");
        match("eql");
        parseType();
        match("semicolon");
    } while (check("ident"));
    indentLevel--;
}

void Parser::parseType() {
    printNode("<type>");
    indentLevel++;

    if (check("arraysy")) {
        parseArrayType();
    } else if (check("lparent")) {
        parseEnumerated();
    } else if (check("recordsy")) {
        parseRecordType();
    } else if (check("intcon") || check("realcon") || check("charcon") ||
               check("string") || check("plus")  || check("minus")) {
        parseRange();
    } else if (check("ident")) {
        if (lookahead(1).type == "period" && lookahead(2).type == "period") {
            parseRange();
        } else {
            match("ident");
        }
    } else {
        std::cerr << "[SYNTAX ERROR] Expected type, got '"
                  << currentToken().type << "'\n";
        exit(1);
    }
    indentLevel--;
}

void Parser::parseArrayType() {
    printNode("<array-type>");
    indentLevel++;
    match("arraysy");
    match("lbrack");

    if (check("ident") && lookahead(1).type != "period") {
        match("ident"); 
    } else {
        parseRange();
    }

    match("rbrack");
    match("ofsy");
    parseType();
    indentLevel--;
}

void Parser::parseRange() {
    printNode("<range>");
    indentLevel++;
    parseConstant();
    match("period");
    match("period");
    parseConstant();
    indentLevel--;
}

void Parser::parseEnumerated() {
    printNode("<enumerated>");
    indentLevel++;
    match("lparent");
    match("ident");
    while (check("comma")) {
        match("comma");
        match("ident");
    }
    match("rparent");
    indentLevel--;
}

void Parser::parseRecordType() {
    printNode("<record-type>");
    indentLevel++;
    match("recordsy");
    parseFieldList();
    match("endsy");
    indentLevel--;
}

void Parser::parseFieldList() {
    printNode("<field-list>");
    indentLevel++;
    parseFieldPart();
    while (check("semicolon")) {
        match("semicolon");
        if (check("endsy")) break; 
        parseFieldPart();
    }
    indentLevel--;
}

void Parser::parseFieldPart() {
    printNode("<field-part>");
    indentLevel++;
    parseIdentifierList();
    match("colon");
    parseType();
    indentLevel--;
}

void Parser::parseVarDeclaration() {
    printNode("<var-declaration>");
    indentLevel++;
    match("varsy");
    do {
        parseIdentifierList();
        match("colon");
        parseType();
        match("semicolon");
    } while (check("ident"));
    indentLevel--;
}

void Parser::parseIdentifierList() {
    printNode("<identifier-list>");
    indentLevel++;
    match("ident");
    while (check("comma")) {
        match("comma");
        match("ident");
    }
    indentLevel--;
}

void Parser::parseSubprogramDeclaration() {
    printNode("<subprogram-declaration>");
    indentLevel++;
    if (check("proceduresy")) parseProcedureDeclaration();
    else                      parseFunctionDeclaration();
    indentLevel--;
}

void Parser::parseProcedureDeclaration() {
    printNode("<procedure-declaration>");
    indentLevel++;
    match("proceduresy");
    match("ident");
    if (check("lparent")) parseFormalParameterList();
    match("semicolon");
    parseBlock();
    match("semicolon");
    indentLevel--;
}

void Parser::parseFunctionDeclaration() {
    printNode("<function-declaration>");
    indentLevel++;
    match("functionsy");
    match("ident");
    if (check("lparent")) parseFormalParameterList();
    match("colon");
    match("ident"); 
    match("semicolon");
    parseBlock();
    match("semicolon");
    indentLevel--;
}

void Parser::parseBlock() {
    printNode("<block>");
    indentLevel++;
    parseDeclarationPart();
    parseCompoundStatement();
    indentLevel--;
}

void Parser::parseFormalParameterList() {
    printNode("<formal-parameter-list>");
    indentLevel++;
    match("lparent");
    parseParameterGroup();
    while (check("semicolon")) {
        match("semicolon");
        parseParameterGroup();
    }
    match("rparent");
    indentLevel--;
}

void Parser::parseParameterGroup() {
    printNode("<parameter-group>");
    indentLevel++;
    parseIdentifierList();
    match("colon");
    if (check("arraysy")) parseArrayType();
    else                  match("ident");
    indentLevel--;
}

void Parser::parseCompoundStatement() {
    printNode("<compound-statement>");
    indentLevel++;
    match("beginsy");
    parseStatementList();
    match("endsy");
    indentLevel--;
}

void Parser::parseStatementList() {
    printNode("<statement-list>");
    indentLevel++;
    if (!check("endsy") && !check("untilsy")) {
        parseStatement();
    }
    while (check("semicolon")) {
        match("semicolon");
        if (check("endsy") || check("untilsy")) {
            break;
        }
        parseStatement();
    }
    indentLevel--;
}

void Parser::parseStatement() {
    printNode("<statement>");
    indentLevel++;

    std::string t = currentToken().type;

    if (t == "ident") {
        std::string next = lookahead(1).type;

        if (next == "lparent") {
            parseProcFuncCall();
        } else if (next == "becomes" || next == "lbrack" || next == "period") {
            parseAssignmentStatement();
        } else {
            if (next == "semicolon" || next == "endsy" || next == "elsesy" || next == "untilsy") {
            } else {
                parseAssignmentStatement();
            }
        }
    } else if (t == "ifsy") {
        parseIfStatement();
    } else if (t == "casesy") {
        parseCaseStatement();
    } else if (t == "whilesy") {
        parseWhileStatement();
    } else if (t == "repeatsy") {
        parseRepeatStatement();
    } else if (t == "forsy") {
        parseForStatement();
    } else if (t == "beginsy") {
        parseCompoundStatement();
    }

    indentLevel--;
}

void Parser::parseVariable() {
    printNode("<variable>");
    indentLevel++;
    match("ident");
    while (check("lbrack") || check("period")) {
        parseComponentVariable();
    }
    indentLevel--;
}

void Parser::parseComponentVariable() {
    printNode("<component-variable>");
    indentLevel++;
    if (check("lbrack")) {
        match("lbrack");
        parseIndexList();
        match("rbrack");
    } else if (check("period")) {
        match("period");
        match("ident");
    }
    indentLevel--;
}

void Parser::parseIndexList() {
    printNode("<index-list>");
    indentLevel++;

    if (check("intcon"))       match("intcon");
    else if (check("charcon")) match("charcon");
    else if (check("ident"))   match("ident");
    else {
        std::cerr << "[SYNTAX ERROR] Expected index, got '"
                  << currentToken().type << "'\n";
        exit(1);
    }

    while (check("comma")) {
        match("comma");
        parseIndexList();
    }
    indentLevel--;
}

void Parser::parseAssignmentStatement() {
    printNode("<assignment-statement>");
    indentLevel++;
    parseVariable();
    match("becomes");
    parseExpression();
    indentLevel--;
}

void Parser::parseIfStatement() {
    printNode("<if-statement>");
    indentLevel++;
    match("ifsy");
    parseExpression();
    match("thensy");
    parseStatement();
    if (check("elsesy")) {
        match("elsesy");
        parseStatement();
    }
    indentLevel--;
}

void Parser::parseCaseStatement() {
    printNode("<case-statement>");
    indentLevel++;
    match("casesy");
    parseExpression();
    match("ofsy");
    parseCaseBlock();
    match("endsy");
    indentLevel--;
}

void Parser::parseCaseBlock() {
    printNode("<case-block>");
    indentLevel++;

    parseConstant();
    while (check("comma")) {
        match("comma");
        parseConstant();
    }
    match("colon");
    parseStatement();

    while (check("semicolon")) {
        match("semicolon");
        if (check("endsy")) break;
        parseCaseBlock();
    }
    indentLevel--;
}

void Parser::parseWhileStatement() {
    printNode("<while-statement>");
    indentLevel++;
    match("whilesy");
    parseExpression();
    match("dosy");
    parseStatement();
    indentLevel--;
}

void Parser::parseRepeatStatement() {
    printNode("<repeat-statement>");
    indentLevel++;
    match("repeatsy");
    parseStatementList();
    match("untilsy");
    parseExpression();
    indentLevel--;
}

void Parser::parseForStatement() {
    printNode("<for-statement>");
    indentLevel++;
    match("forsy");
    match("ident");
    match("becomes");
    parseExpression();
    if (check("tosy"))        match("tosy");
    else if (check("downtosy")) match("downtosy");
    else {
        std::cerr << "[SYNTAX ERROR] Expected 'to' or 'downto', got '"
                  << currentToken().type << "'\n";
        exit(1);
    }
    parseExpression();
    match("dosy");
    parseStatement();
    indentLevel--;
}

void Parser::parseProcFuncCall() {
    printNode("<procedure/function-call>");
    indentLevel++;
    match("ident");
    if (check("lparent")) {
        match("lparent");
        if (!check("rparent")) {
            parseParameterList();
        }
        match("rparent");
    }
    indentLevel--;
}

void Parser::parseParameterList() {
    printNode("<parameter-list>");
    indentLevel++;
    parseExpression();
    while (check("comma")) {
        match("comma");
        parseExpression();
    }
    indentLevel--;
}

void Parser::parseExpression() {
    printNode("<expression>");
    indentLevel++;
    parseSimpleExpression();
    if (isRelationalOp()) {
        printNode("<relational-operator>");
        indentLevel++;
        match(currentToken().type);
        indentLevel--;
        parseSimpleExpression();
    }
    indentLevel--;
}

void Parser::parseSimpleExpression() {
    printNode("<simple-expression>");
    indentLevel++;
    if (check("plus"))       match("plus");
    else if (check("minus")) match("minus");

    parseTerm();
    while (isAdditiveOp()) {
        printNode("<additive-operator>");
        indentLevel++;
        match(currentToken().type);
        indentLevel--;
        parseTerm();
    }
    indentLevel--;
}

void Parser::parseTerm() {
    printNode("<term>");
    indentLevel++;
    parseFactor();
    while (isMultiplecativeOp()) {
        printNode("<multiplicative-operator>");
        indentLevel++;
        match(currentToken().type);
        indentLevel--;
        parseFactor();
    }
    indentLevel--;
}

void Parser::parseFactor() {
    printNode("<factor>");
    indentLevel++;

    if (check("intcon")) {
        match("intcon");
    } else if (check("realcon")) {
        match("realcon");
    } else if (check("charcon")) {
        match("charcon");
    } else if (check("string")) {
        match("string");
    } else if (check("lparent")) {
        match("lparent");
        parseExpression();
        match("rparent");
    } else if (check("notsy")) {
        match("notsy");
        parseFactor();
    } else if (check("ident")) {
        if (lookahead(1).type == "lparent") {
            parseProcFuncCall();
        } else {
            parseVariable();
        }
    } else {
        std::cerr << "[SYNTAX ERROR] Unexpected token in factor: '"
                  << currentToken().type << "'\n";
        exit(1);
    }
    indentLevel--;
}
