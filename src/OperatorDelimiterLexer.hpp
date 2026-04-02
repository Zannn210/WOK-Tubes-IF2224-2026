// OperatorDelimiterLexer.hpp
#ifndef OPERATOR_DELIMITER_LEXER_HPP
#define OPERATOR_DELIMITER_LEXER_HPP

#include <string>

class OperatorDelimiterLexer {
private:
    std::string buffer;
    int state;
    
    bool isLetter(char c);
    bool isDigit(char c);
    char toLower(char c);

public:
    OperatorDelimiterLexer();

    void reset();

    bool processChar(char c);
    std::string getToken();
    bool canStart(char c);
    std::string getBuffer();
    bool isDone();
};

#endif