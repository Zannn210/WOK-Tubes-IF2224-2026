#include "OperatorDelimiterLexer.hpp"

OperatorDelimiterLexer::OperatorDelimiterLexer() {
    state = 0;
    buffer = "";
}

void OperatorDelimiterLexer::reset() {
    state = 0;
    buffer = "";
}

bool OperatorDelimiterLexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool OperatorDelimiterLexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

char OperatorDelimiterLexer::toLower(char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

bool OperatorDelimiterLexer::processChar(char c) {

    char lc = toLower(c);

    // state 0
    if (state == 0) {
        buffer += c;

        if (c == '+') state = 20001;
        else if (c == '-') state = 20002;
        else if (c == '*') state = 20003;
        else if (c == '/') state = 20004;

        else if (c == '=') state = 20010;
        else if (c == '<') state = 20020;
        else if (c == '>') state = 20030;

        else if (c == '(') state = 20040;
        else if (c == ')') state = 20041;
        else if (c == '[') state = 20042;
        else if (c == ']') state = 20043;

        else if (c == ',') state = 20044;
        else if (c == ';') state = 20045;
        else if (c == '.') state = 20046;

        else if (c == ':') state = 20050;

        // word operators
        else if (lc == 'd') state = 21010;
        else if (lc == 'm') state = 21020;
        else if (lc == 'a') state = 21030;
        else if (lc == 'o') state = 21040;
        else if (lc == 'n') state = 21050;

        else return false;

        return true;
    }

    // ===== DIV =====
    if (state == 21010) { // "d"
        buffer += c;
        if (lc == 'i') state = 21011; 
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21011) { // "di"
        buffer += c;
        if (lc == 'v') state = 21012;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21012) { // "div"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 21013;
        return false;
    }

    // ===== MOD =====
    if (state == 21020) { // "m"
        buffer += c;
        if (lc == 'o') state = 21021;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21021) { // "mo"
        buffer += c;
        if (lc == 'd') state = 21022;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21022) { // "mod"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 21023;
        return false;
    }

    // ===== AND =====
    if (state == 21030) { // "a"
        buffer += c;
        if (lc == 'n') state = 21031;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21031) { // "an"
        buffer += c;
        if (lc == 'd') state = 21032;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21032) { // "and"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 21033;
        return false;
    }

    // ===== OR =====
    if (state == 21040) { // "o"
        buffer += c;
        if (lc == 'r') state = 21041;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21041) { // "or"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 21042;
        return false;
    }

    // ===== NOT =====
    if (state == 21050) { // "n"
        buffer += c;
        if (lc == 'o') state = 21051;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21051) { // "no"
        buffer += c;
        if (lc == 't') state = 21052;
        else if (isLetter(c) || isDigit(c)) state = 9999;
        else return false;
        return true;
    }

    if (state == 21052) { // "not"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 21053;
        return false;
    }

    // ===== IDENTIFIER FALLBACK =====
    if (state == 9999) {
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            return true;
        }
        state = 10000;
        return false;
    }

    // ===== SYMBOL HANDLING =====
    if (state == 20010) { // "="
        if (c == '=') { // "=="
            buffer += c;
            state = 20011;
            return true;
        }
        return false;
    }

    if (state == 20020) { // "<"
        if (c == '=') { // "<="
            buffer += c;
            state = 20021;
            return true;
        } else if (c == '>') { // "<>"
            buffer += c;
            state = 20022;
            return true;
        }
        return false;
    }

    if (state == 20030) { // >
        if (c == '=') { // ">=
            buffer += c;
            state = 20031;
            return true;
        }
        return false;
    }

    if (state == 20050) { // ":"
        if (c == '=') { // ":="
            buffer += c;
            state = 20051;
            return true;
        }
        return false;
    }

    return false;
}

std::string OperatorDelimiterLexer::getToken() {

    if (state == 20001) return "plussy";
    if (state == 20002) return "minussy";
    if (state == 20003) return "timessy";
    if (state == 20004) return "rdivsy";

    if (state == 20011) return "eqlsy";
    if (state == 20022) return "neqsy";
    if (state == 20030) return "gtrsy";
    if (state == 20031) return "geqsy";
    if (state == 20020) return "lsssy";
    if (state == 20021) return "leqsy";

    if (state == 20040) return "lparentsy";
    if (state == 20041) return "rparentsy";
    if (state == 20042) return "lbracksy";
    if (state == 20043) return "rbracksy";

    if (state == 20044) return "commasy";
    if (state == 20045) return "semicolonsy";
    if (state == 20046) return "periodsy";

    if (state == 20050) return "colonsy";
    if (state == 20051) return "becomessy";

    if (state == 21013) return "idivsy";
    if (state == 21023) return "imodsy";
    if (state == 21033) return "andsy";
    if (state == 21042) return "orsy";
    if (state == 21053) return "notsy";

    if (state == 10000) return "ident (" + buffer + ")";

    return "";
}

bool OperatorDelimiterLexer::canStart(char c) {
    char lc = toLower(c);
    return std::string("+-*/=<>()[];,.:").find(c) != std::string::npos ||
           lc == 'd' || lc == 'm' || lc == 'a' || lc == 'o' || lc == 'n';
}

std::string OperatorDelimiterLexer::getBuffer() {
    return buffer;
}

bool OperatorDelimiterLexer::isDone() {
    return (state >= 20001 && state != 9999 && state != 0);
}