#include "ArionLexer.hpp"
#include <iostream>
#include <cctype>


enum LexerState {
    STATE_START,  // state awal 
    STATE_IDENT_KW, // sedang membaca identifier/keyword
    STATE_NUMBER, // sedang membaca integer atau realcon
    STATE_STRING, // sedang membaca string atau charcon
    STATE_COMMENT_CU, // sedang membaca komentar {}
    STATE_PAREN,// sudah baca '(', cek karakter berikutnya
    STATE_COMMENT_ST,// sedang membaca komentar (**)
    STATE_OPERATOR,// sedang membaca operator/delimiter
    STATE_ERROR // karakter tidak dikenal → unknown, balik ke awal
};

std::string normalizeToken(const std::string& raw) {
    if (raw.empty()) return "";

    auto addSpace = [](const std::string& s) -> std::string {
        size_t p = s.find('(');
        if (p != std::string::npos && p > 0 && s[p - 1] != ' ')
            return s.substr(0, p) + " " + s.substr(p);
        return s;
    };

    if (raw.rfind("intcon(",   0) == 0) return addSpace(raw);
    if (raw.rfind("realcon(",  0) == 0) return addSpace(raw);
    if (raw.rfind("charcon(",  0) == 0) return addSpace(raw);
    if (raw.rfind("string(",   0) == 0) return addSpace(raw);
    if (raw.rfind("ident (",   0) == 0) return raw;
    if (raw.rfind("ident(",    0) == 0) return addSpace(raw);
    if (raw.rfind("comment(",  0) == 0) return addSpace(raw); 
    if (raw.rfind("unknown(",  0) == 0) return addSpace(raw);

    if (raw == "plussy")      return "plus";
    if (raw == "minussy")     return "minus";
    if (raw == "timessy")     return "times";
    if (raw == "rdivsy")      return "rdiv";
    if (raw == "eqlsy")       return "eql";
    if (raw == "neqsy")       return "neq";
    if (raw == "gtrsy")       return "gtr";
    if (raw == "geqsy")       return "geq";
    if (raw == "lsssy")       return "lss";
    if (raw == "leqsy")       return "leq";
    if (raw == "lparentsy")   return "lparent";
    if (raw == "rparentsy")   return "rparent";
    if (raw == "lbracksy")    return "lbrack";
    if (raw == "rbracksy")    return "rbrack";
    if (raw == "commasy")     return "comma";
    if (raw == "semicolonsy") return "semicolon\n";
    if (raw == "periodsy")    return "period";
    if (raw == "colonsy")     return "colon";
    if (raw == "becomessy")   return "becomes";
    if (raw == "idivsy")      return "idiv";
    if (raw == "imodsy")      return "imod";

    return raw;
}


bool ArionLexer::isDigit(char c)  { return c >= '0' && c <= '9'; }
bool ArionLexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
char ArionLexer::toLower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

std::string ArionLexer::mapKeyword(const std::string& w) {
    if (w == "const")     return "constsy";
    if (w == "type")      return "typesy";
    if (w == "var")       return "varsy";
    if (w == "function")  return "functionsy";
    if (w == "procedure") return "proceduresy";
    if (w == "array")     return "arraysy";
    if (w == "record")    return "recordsy";
    if (w == "program")   return "programsy";
    if (w == "begin")     return "beginsy";
    if (w == "if")        return "ifsy";
    if (w == "case")      return "casesy";
    if (w == "repeat")    return "repeatsy";
    if (w == "while")     return "whilesy";
    if (w == "for")       return "forsy";
    if (w == "end")       return "endsy";
    if (w == "else")      return "elsesy";
    if (w == "until")     return "untilsy";
    if (w == "of")        return "ofsy";
    if (w == "do")        return "dosy";
    if (w == "to")        return "tosy";
    if (w == "downto")    return "downtosy";
    if (w == "then")      return "thensy";
    if (w == "not")       return "notsy";
    if (w == "and")       return "andsy";
    if (w == "or")        return "orsy";
    if (w == "div")       return "idivsy";
    if (w == "mod")       return "imodsy";
    return "";
}

void ArionLexer::emit(const std::string& raw) {
    std::string cleaned;
    for (char ch : raw)
    if (ch != '\r') cleaned += ch;
    std::string result = normalizeToken(raw);
    if (result.empty()) return;
    std::cout << result << "\n";
    if (hasOutputFile) outputFile << result << "\n";
}

//constructor
ArionLexer::ArionLexer(const std::string& inputPath,
                       const std::string& outputPath) {
    inputFile.open(inputPath);
    hasOutputFile = !outputPath.empty();
    if (hasOutputFile) outputFile.open(outputPath);
}

//destructor
ArionLexer::~ArionLexer() {
    if (inputFile.is_open())  inputFile.close();
    if (outputFile.is_open()) outputFile.close();
}

//buka file
bool ArionLexer::isOpen() const { return inputFile.is_open(); }


// analyze: loop utama DFA
void ArionLexer::analyze() {
    if (!inputFile.is_open()) {
        std::cerr << "[ERROR] File input tidak dapat dibuka.\n";
        return;
    }

    InputBuffer inBuf(inputFile);
    LexerState  state = STATE_START;

    while (true) {
        if (!inBuf.hasBuffer && inputFile.eof()) break;

        char c = inBuf.get();

        if (c== '\r')
        {
            continue;
        }
        
        switch (state) {

 
        case STATE_START: {

            if (std::isspace((unsigned char)c)) {
                state = STATE_START;
                break;
            }

            if (isLetter(c)) {
                state = STATE_IDENT_KW;

                kwLexer.reset();
                kwLexer.processChar(c);

                std::string word(1, c);
                std::string lw(1, toLower(c));

                while (true) {
                    if (!inBuf.hasBuffer && inputFile.eof()) break;
                    char next = inBuf.get();
                    bool consumed = kwLexer.processChar(next);
                    if (consumed) {
                        word += next;
                        lw   += toLower(next);
                    } else {
                        inBuf.put(next);
                        break;
                    }
                }

                kwLexer.processChar(' ');

                std::string kwResult = mapKeyword(lw);
                if (!kwResult.empty()) {
                    emit(kwResult);
                } else {
                    emit("ident (" + word + ")");
                }
                state = STATE_START;
                break;
            }

            if (isDigit(c)) {
                state = STATE_NUMBER;
                emit(numHandler.process(inBuf, c));
                state = STATE_START;
                break;
            }

            if (c == '\'') {
                state = STATE_STRING;
                emit(strHandler.process(inBuf, c));
                state = STATE_START;
                break;
            }

            if (c == '{') {
                state = STATE_COMMENT_CU;
                emit(comHandler.process(inBuf, c));
                state = STATE_START;
                break;
            }

            if (c == '(') {
                state = STATE_PAREN;
                if (!inBuf.hasBuffer && inputFile.eof()) {
                    emit("lparentsy");
                } else {
                    char next = inBuf.get();
                    if (next == '*') {
                        inBuf.put(next);
                        state = STATE_COMMENT_ST;
                        emit(comHandler.process(inBuf, c));
                    } else {
                        inBuf.put(next);
                        emit("lparentsy");
                    }
                }
                state = STATE_START;
                break;
            }

            if (opLexer.canStart(c)) {
                state = STATE_OPERATOR;
                opLexer.reset();
                opLexer.processChar(c);

                bool needPeek = (c == ':' || c == '=' ||
                                 c == '<' || c == '>');
                if (needPeek && !(!inBuf.hasBuffer && inputFile.eof())) {
                    char next = inBuf.get();
                    bool consumed = opLexer.processChar(next);
                    if (!consumed) inBuf.put(next);
                }

                std::string token = opLexer.getToken();
                if (!token.empty()) {
                    emit(token);
                } else {
                    std::cerr << "[LEXICAL ERROR] Token tidak valid: '" << c << "'\n";
                    std::string unk = "unknown (" + std::string(1, c) + ")";
                    std::cout << unk << "\n";
                    if (hasOutputFile) outputFile << unk << "\n";
                }
                state = STATE_START;
                break;
            }

            state = STATE_ERROR;
            state = STATE_START;
            break;
        }

        default:
            state = STATE_START;
            inBuf.put(c);
            break;
        }
    }
}
