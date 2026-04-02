#include "KeywordIdentifierLexer.hpp"

#include <cctype>

void KeywordIdentifierLexer::initializeKeywords() {
    keywords["const"] = "constsy";
    keywords["type"] = "typesy";
    keywords["var"] = "varsy";
    keywords["function"] = "functionsy";
    keywords["procedure"] = "proceduresy";
    keywords["array"] = "arraysy";
    keywords["record"] = "recordsy";
    keywords["program"] = "programsy";
    keywords["begin"] = "beginsy";
    keywords["if"] = "ifsy";
    keywords["case"] = "casesy";
    keywords["repeat"] = "repeatsy";
    keywords["while"] = "whilesy";
    keywords["for"] = "forsy";
    keywords["end"] = "endsy";
    keywords["else"] = "elsesy";
    keywords["until"] = "untilsy";
    keywords["of"] = "ofsy";
    keywords["do"] = "dosy";
    keywords["to"] = "tosy";
    keywords["downto"] = "downtosy";
    keywords["then"] = "thensy";
}

std::string KeywordIdentifierLexer::toLowerCase(const std::string& str) {
    std::string result = str;
    for (int i = 0; i < static_cast<int>(result.length()); i++) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}

bool KeywordIdentifierLexer::isLetter(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        return true;
    }
    return false;
}

bool KeywordIdentifierLexer::isDigit(char c) {
    if (c >= '0' && c <= '9') {
        return true;
    }
    return false;
}

KeywordIdentifierLexer::KeywordIdentifierLexer() {
    state = 0;
    initializeKeywords();
}

void KeywordIdentifierLexer::reset() {
    buffer = "";
    state = 0;
}

bool KeywordIdentifierLexer::processChar(char c) {
    if (state == 0) {
        if (isLetter(c)) {
            buffer = buffer + c;
            state = 1;
            return true;
        }
        return false;
    }

    if (state == 1) {
        if (isLetter(c) || isDigit(c)) {
            buffer = buffer + c;
            return true;
        }
        state = 2;
        return false;
    }

    return false;
}

std::string KeywordIdentifierLexer::getToken() {
    if (buffer == "") {
        return "";
    }

    std::string lowerBuffer = toLowerCase(buffer);

    // Cek apakah keyword
    if (keywords.find(lowerBuffer) != keywords.end()) {
        return keywords[lowerBuffer];
    }

    // Bukan keyword, berarti identifier
    return "ident (" + buffer + ")";
}

bool KeywordIdentifierLexer::canStart(char c) {
    return isLetter(c);
}

std::string KeywordIdentifierLexer::getBuffer() {
    return buffer;
}

bool KeywordIdentifierLexer::isDone() {
    if (state == 2) {
        return true;
    }
    return false;
}
