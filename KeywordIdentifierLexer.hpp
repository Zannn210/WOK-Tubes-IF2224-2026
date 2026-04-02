// KeywordIdentifierLexer.hpp
#ifndef KEYWORD_IDENTIFIER_LEXER_HPP
#define KEYWORD_IDENTIFIER_LEXER_HPP

#include <string>
#include <unordered_map>

class KeywordIdentifierLexer {
private:
    std::unordered_map<std::string, std::string> keywords;
    std::string buffer;
    int state;

    void initializeKeywords();
    std::string toLowerCase(const std::string& str);
    bool isLetter(char c);
    bool isDigit(char c);

public:
    KeywordIdentifierLexer();

    void reset();

    // Proses karakter satu per satu (DFA)
    bool processChar(char c);
    std::string getToken();
    bool canStart(char c);
    std::string getBuffer();
    bool isDone();
};

#endif