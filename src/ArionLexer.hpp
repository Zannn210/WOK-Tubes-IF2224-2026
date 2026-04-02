#ifndef ARION_LEXER_HPP
#define ARION_LEXER_HPP

#include <fstream>
#include <string>

#include "KeywordIdentifierLexer.hpp"
#include "TipeDataKomentar.hpp"
#include "OperatorDelimiterLexer.hpp"

std::string normalizeToken(const std::string& raw);

class ArionLexer {
private:
    std::ifstream inputFile;
    std::ofstream outputFile;
    bool          hasOutputFile;

    KeywordIdentifierLexer kwLexer;
    NumberHandler          numHandler;
    StringHandler          strHandler;
    CommentHandler         comHandler;
    OperatorDelimiterLexer opLexer;

    bool isDigit(char c);
    bool isLetter(char c);
    char toLower(char c);

    std::string mapKeyword(const std::string& lowerWord);

    void emit(const std::string& raw);


public:
    ArionLexer(const std::string& inputPath, const std::string& outputPath);
    ~ArionLexer();

    bool isOpen() const;

    void analyze();
};

#endif
