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

void Parser::printNode(const std::string& nodeName) {
    std::string indent = "";
    for(int i = 0; i < indentLevel; i++) indent += "|   ";
    if (indentLevel > 0) indent += "|-- ";

    std::cout << indent << nodeName << "\n";
    if (outFile.is_open()) {
        outFile << indent << nodeName << "\n";
    }
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
        std::cerr << "\n[SYNTAX ERROR] Unexpected token '" << t.type 
                  << "', expected '" << expectedType << "'\n";
        exit(1); 
    }
}

void Parser::parse() {
    parseProgram();
}

//root program

void Parser::parseProgram() {
    printNode("<program>");
    indentLevel++;
    
    parseProgramHeader();
    // parseDeclarationPart(); 
    // parseCompoundStatement(); 
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

// --- IMPLEMENTASI SYNTAX-DIRECTED TRANSLATION ---

std::shared_ptr<StatementNode> Parser::parseStatement() {
    printNode("<statement>");
    indentLevel++;
    
    std::shared_ptr<StatementNode> stmtNode = nullptr;
    
    if (check("ident")) {
        stmtNode = parseAssignmentStatement();
    }
    // Bisa ditambahkan: IfNode, WhileNode, ForNode, dll
    
    indentLevel--;
    return stmtNode;
}

std::shared_ptr<StatementNode> Parser::parseAssignmentStatement() {
    printNode("<assignment-statement>");
    indentLevel++;

    std::string varName = currentToken().value;
    match("ident");
    match("becomes");
    
    std::shared_ptr<ExprNode> exprNode = parseExpression();
    
    indentLevel--;
    
    // Membentuk node AST dan mengembalikannya
    auto targetNode = std::make_shared<VarNode>(varName);
    return std::make_shared<AssignNode>(targetNode, exprNode);
}

std::shared_ptr<ExprNode> Parser::parseExpression() {
    printNode("<expression>");
    indentLevel++;
    
    std::shared_ptr<ExprNode> leftNode = parseSimpleExpression();
    
    if (isRelationalOp()) {
        std::string op = currentToken().type;
        match(op);
        std::shared_ptr<ExprNode> rightNode = parseSimpleExpression();
        leftNode = std::make_shared<BinOpNode>(op, leftNode, rightNode);
    }
    
    indentLevel--;
    return leftNode;
}

std::shared_ptr<ExprNode> Parser::parseSimpleExpression() {
    printNode("<simple-expression>");
    indentLevel++;
    
    // Handle unary plus/minus (optional, untuk penyederhanaan diasumsikan ada)
    if (check("plus")) {
        match("plus");
    } else if (check("minus")) {
        match("minus");
    }

    std::shared_ptr<ExprNode> leftNode = parseTerm();
    
    while (isAdditiveOp()) {
        std::string op = currentToken().type;
        match(op);
        std::shared_ptr<ExprNode> rightNode = parseTerm();
        leftNode = std::make_shared<BinOpNode>(op, leftNode, rightNode);
    }
    
    indentLevel--;
    return leftNode;
}

std::shared_ptr<ExprNode> Parser::parseTerm() {
    printNode("<term>");
    indentLevel++;
    
    std::shared_ptr<ExprNode> leftNode = parseFactor();
    
    while (isMultiplicativeOp()) {
        std::string op = currentToken().type;
        match(op);
        std::shared_ptr<ExprNode> rightNode = parseFactor();
        leftNode = std::make_shared<BinOpNode>(op, leftNode, rightNode);
    }
    
    indentLevel--;
    return leftNode;
}

std::shared_ptr<ExprNode> Parser::parseFactor() {
    printNode("<factor>");
    indentLevel++;
    
    std::shared_ptr<ExprNode> node = nullptr;

    if (check("intcon")) {
        node = std::make_shared<NumberNode>(currentToken().value, "intcon");
        match("intcon");
    } else if (check("realcon")) {
        node = std::make_shared<NumberNode>(currentToken().value, "realcon");
        match("realcon");
    } else if (check("string")) {
        node = std::make_shared<StringNode>(currentToken().value);
        match("string");
    } else if (check("ident")) {
        if (lookahead(1).type == "lparent") {
            // Proc/func call handling - untuk sekarang return VarNode saja
            node = std::make_shared<VarNode>(currentToken().value);
            match("ident");
        } else {
            node = std::make_shared<VarNode>(currentToken().value);
            match("ident");
        }
    } else if (check("lparent")) {
        match("lparent");
        node = parseExpression();
        match("rparent");
    }
    
    indentLevel--;
    return node;
}

std::shared_ptr<ExprNode> Parser::parseVariable() {
    printNode("<variable>");
    indentLevel++;
    
    std::string varName = currentToken().value;
    match("ident");
    
    indentLevel--;
    return std::make_shared<VarNode>(varName);
}

// --- FUNGSI HELPER UNTUK PENGECEKAN OPERATOR ---

bool Parser::isRelationalOp() {
    std::string type = currentToken().type;
    return type == "eq" || type == "neq" || type == "lt" || 
           type == "lte" || type == "gt" || type == "gte";
}

bool Parser::isAdditiveOp() {
    std::string type = currentToken().type;
    return type == "plus" || type == "minus" || type == "or";
}

bool Parser::isMultiplicativeOp() {
    std::string type = currentToken().type;
    return type == "times" || type == "divide" || type == "mod" || type == "and";
}

// Fungsi helper untuk memeriksa token saat ini
bool Parser::check(const std::string& expectedType) {
    return currentToken().type == expectedType;
}