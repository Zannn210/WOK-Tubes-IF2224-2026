#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <fstream>

// Struktur data untuk menyimpan Token dari Lexer
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

    // Fungsi utilitas internal
    Token currentToken();
    Token lookahead(int offset);
    void advance();
    void printNode(const std::string& nodeName);
    bool check(const std::string& type);
    bool isRelationalOp();
    bool isAdditiveOp();
    bool isMultiplecativeOp();

public:
    Parser(const std::vector<Token>& t, const std::string& outFilename);
    ~Parser();

    // Fungsi utama Error Handling
    bool match(const std::string& expectedType);

    // Entry point parser
    void parse();

    void parseProgram();
    void parseProgramHeader();
    void parseDeclarationPart();
    void parseBlock();
    void parseConstDeclaration();
    void parseConstant();
    void parseTypeDeclaration();
    void parseType();
    void parseArrayType();
    void parseRange();
    void parseEnumerated();
    void parseRecordType();
    void parseFieldList();
    void parseFieldPart();
    void parseVarDeclaration();
    void parseIdentifierList();
    void parseSubprogramDeclaration();
    void parseProcedureDeclaration();
    void parseFunctionDeclaration();
    void parseFormalParameterList();
    void parseParameterGroup();
    void parseCompoundStatement();
    void parseStatementList();
    void parseStatement();
    void parseAssignmentStatement();
    void parseIfStatement();
    void parseCaseStatement();
    void parseCaseBlock();
    void parseWhileStatement();
    void parseRepeatStatement();
    void parseForStatement();
    void parseProcFuncCall();
    void parseVariable();
    void parseComponentVariable();
    void parseIndexList();
    void parseParameterList();
    void parseExpression();
    void parseSimpleExpression();
    void parseTerm();
    void parseFactor();

};

#endif