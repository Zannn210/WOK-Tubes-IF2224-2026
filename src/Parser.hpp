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

public:
    Parser(const std::vector<Token>& t, const std::string& outFilename);
    ~Parser();

    // Fungsi utama Error Handling
    bool match(const std::string& expectedType);

    // Entry point parser
    void parse();

    void parseProgram();
    void parseProgramHeader();

    //di bawah diisi fungsi anggota2 lain

};

#endif