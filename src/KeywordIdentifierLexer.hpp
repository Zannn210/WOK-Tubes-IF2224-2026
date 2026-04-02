// KeywordIdentifierLexer.hpp
#ifndef KEYWORD_IDENTIFIER_LEXER_HPP
#define KEYWORD_IDENTIFIER_LEXER_HPP

#include <string>

class KeywordIdentifierLexer {
private:
    std::string buffer;
    int state;
    
    bool isLetter(char c);
    bool isDigit(char c);
    char toLower(char c);

public:
    KeywordIdentifierLexer();

    void reset();

    bool processChar(char c);
    std::string getToken();
    bool canStart(char c);
    std::string getBuffer();
    bool isDone();
};

#endif
