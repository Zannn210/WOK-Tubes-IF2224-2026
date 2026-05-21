#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "ASTNode.hpp"

// Token produced by the lexer and consumed by the parser
struct Token {
    std::string type;
    std::string value;
};

std::vector<Token> loadTokensFromFile(const std::string& filename);

class Parser {
private:
    std::vector<Token> tokens;
    int currentIndex;
    int indentLevel;
    std::ofstream outFile;

    Token currentToken();
    Token lookahead(int offset);
    void  advance();
    void  printNode(const std::string& nodeName);
    bool  check(const std::string& type);
    bool  isRelationalOp();
    bool  isAdditiveOp();
    bool  isMultiplicativeOp();

public:
    Parser(const std::vector<Token>& t, const std::string& outFilename);
    ~Parser();

    // Match expected token; prints it, advances, returns terminal ASTNode
    ASTNode* match(const std::string& expectedType);

    // Entry point — returns the root of the parse tree
    ASTNode* parse();

    // Grammar rules — each returns the corresponding subtree
    ASTNode* parseProgram();
    ASTNode* parseProgramHeader();
    ASTNode* parseDeclarationPart();
    ASTNode* parseBlock();
    ASTNode* parseConstDeclaration();
    ASTNode* parseConstant();
    ASTNode* parseTypeDeclaration();
    ASTNode* parseType();
    ASTNode* parseArrayType();
    ASTNode* parseRange();
    ASTNode* parseEnumerated();
    ASTNode* parseRecordType();
    ASTNode* parseFieldList();
    ASTNode* parseFieldPart();
    ASTNode* parseVarDeclaration();
    ASTNode* parseIdentifierList();
    ASTNode* parseSubprogramDeclaration();
    ASTNode* parseProcedureDeclaration();
    ASTNode* parseFunctionDeclaration();
    ASTNode* parseFormalParameterList();
    ASTNode* parseParameterGroup();
    ASTNode* parseCompoundStatement();
    ASTNode* parseStatementList();
    ASTNode* parseStatement();          // returns nullptr for empty statement
    ASTNode* parseAssignmentStatement();
    ASTNode* parseIfStatement();
    ASTNode* parseCaseStatement();
    ASTNode* parseCaseBlock();
    ASTNode* parseWhileStatement();
    ASTNode* parseRepeatStatement();
    ASTNode* parseForStatement();
    ASTNode* parseProcFuncCall();
    ASTNode* parseVariable();
    ASTNode* parseComponentVariable();
    ASTNode* parseIndexList();
    ASTNode* parseParameterList();
    ASTNode* parseExpression();
    ASTNode* parseSimpleExpression();
    ASTNode* parseTerm();
    ASTNode* parseFactor();
};

#endif // PARSER_HPP
