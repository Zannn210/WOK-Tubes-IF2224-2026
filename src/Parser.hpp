#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "AST.hpp"

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
    bool check(const std::string& expectedType);

public:
    Parser(const std::vector<Token>& t, const std::string& outFilename);
    ~Parser();

    // Fungsi utama Error Handling
    bool match(const std::string& expectedType);

    // Entry point parser
    void parse();

    void parseProgram();
    void parseProgramHeader();

    // Fungsi parsing yang mengembalikan AST nodes (Syntax-Directed Translation)
    std::shared_ptr<StatementNode> parseAssignmentStatement();
    std::shared_ptr<StatementNode> parseStatement();
    std::shared_ptr<ExprNode> parseExpression();
    std::shared_ptr<ExprNode> parseSimpleExpression();
    std::shared_ptr<ExprNode> parseTerm();
    std::shared_ptr<ExprNode> parseFactor();
    std::shared_ptr<ExprNode> parseVariable();

    // Fungsi helper untuk pengecekan operator
    bool isRelationalOp();
    bool isAdditiveOp();
    bool isMultiplicativeOp();

};

#endif