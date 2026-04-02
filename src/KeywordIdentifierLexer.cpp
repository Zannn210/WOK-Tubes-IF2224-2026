#include "KeywordIdentifierLexer.hpp"
#include <cctype>

KeywordIdentifierLexer::KeywordIdentifierLexer() {
    state = 0;
    buffer = "";
}

void KeywordIdentifierLexer::reset() {
    buffer = "";
    state = 0;
}

bool KeywordIdentifierLexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool KeywordIdentifierLexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

char KeywordIdentifierLexer::toLower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

bool KeywordIdentifierLexer::processChar(char c) {
    char lc = toLower(c);
    
    // State 0
    if (state == 0) {
        if (!isLetter(c)) {
            return false;
        }
        buffer += c;
        
        // huruf pertama
        if (lc == 'c') {
            state = 100;
        } else if (lc == 't') {
            state = 200;
        } else if (lc == 'v') {
            state = 300;
        } else if (lc == 'f') {
            state = 400;
        } else if (lc == 'p') {
            state = 500;
        } else if (lc == 'a') {
            state = 600;
        } else if (lc == 'r') {
            state = 700;
        } else if (lc == 'b') {
            state = 800;
        } else if (lc == 'i') {
            state = 900;
        } else if (lc == 'w') {
            state = 1000;
        } else if (lc == 'e') {
            state = 1100;
        } else if (lc == 'u') {
            state = 1200;
        } else if (lc == 'o') {
            state = 1300;
        } else if (lc == 'd') {
            state = 1400;
        } else {
            state = 9999;
        }
        return true;
    }
    
    // ===== JALUR C: const (29), case (40) ====
    if (state == 100) {
        buffer += c;
        if (lc == 'o') {
            state = 101; // "co"
            return true;
        } else if (lc == 'a') {
            state = 110; // "ca"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 101) { // "co"
        buffer += c;
        if (lc == 'n') {
            state = 102;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 102) { // "con"
        buffer += c;
        if (lc == 's') {
            state = 103;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 103) { // "cons"
        buffer += c;
        if (lc == 't') {
            state = 104; // "const"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 104) { // "const"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 105;
        return false;
    }
    
    if (state == 110) { // "ca"
        buffer += c;
        if (lc == 's') {
            state = 111;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 111) { // "cas"
        buffer += c;
        if (lc == 'e') {
            state = 112; // "case"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 112) { // "case"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 113; // Done
        return false;
    }
    
    // ===== JALUR T: type (30), then (51), to (49) =====
    if (state == 200) {
        buffer += c;
        if (lc == 'y') {
            state = 201; // "ty"
            return true;
        } else if (lc == 'h') {
            state = 210; // "th"
            return true;
        } else if (lc == 'o') {
            state = 220; // "to"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 201) { // "ty"
        buffer += c;
        if (lc == 'p') {
            state = 202;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 202) { // "typ"
        buffer += c;
        if (lc == 'e') {
            state = 203; // "type" - FINAL
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 203) { // "type" - FINAL STATE (token 30)
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 204; // Done
        return false;
    }
    
    if (state == 210) { // "th"
        buffer += c;
        if (lc == 'e') {
            state = 211;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 211) { // "the"
        buffer += c;
        if (lc == 'n') {
            state = 212; // "then"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 212) { // "then"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 213;
        return false;
    }
    
    if (state == 220) { // "to"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 221;
        return false;
    }
    
    // ===== JALUR V: var (31) =====
    if (state == 300) {
        buffer += c;
        if (lc == 'a') {
            state = 301;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 301) { // "va"
        buffer += c;
        if (lc == 'r') {
            state = 302; // "var"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 302) { // "var"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 303;
        return false;
    }
    
    // ===== JALUR F: function (32), for (43) =====
    if (state == 400) {
        buffer += c;
        if (lc == 'u') {
            state = 401; // "fu"
            return true;
        } else if (lc == 'o') {
            state = 410; // "fo"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 401) { // "fu"
        buffer += c;
        if (lc == 'n') {
            state = 402;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 402) { // "fun"
        buffer += c;
        if (lc == 'c') {
            state = 403;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 403) { // "func"
        buffer += c;
        if (lc == 't') {
            state = 404;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 404) { // "funct"
        buffer += c;
        if (lc == 'i') {
            state = 405;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 405) { // "functi"
        buffer += c;
        if (lc == 'o') {
            state = 406;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 406) { // "functio"
        buffer += c;
        if (lc == 'n') {
            state = 407; // "function"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 407) { // "function"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 408; // Done
        return false;
    }
    
    if (state == 410) { // "fo"
        buffer += c;
        if (lc == 'r') {
            state = 411; // "for"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 411) { // "for"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 412;
        return false;
    }
    
    // ===== JALUR P: procedure (33), program (36) =====
    if (state == 500) {
        buffer += c;
        if (lc == 'r') {
            state = 501;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 501) { // "pr"
        buffer += c;
        if (lc == 'o') {
            state = 502;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 502) { // "pro"
        buffer += c;
        if (lc == 'c') {
            state = 503;
            return true;
        } else if (lc == 'g') {
            state = 510;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 503) { // "proc"
        buffer += c;
        if (lc == 'e') {
            state = 504;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 504) { // "proce"
        buffer += c;
        if (lc == 'd') {
            state = 505;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 505) { // "proced"
        buffer += c;
        if (lc == 'u') {
            state = 506;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 506) { // "procedu"
        buffer += c;
        if (lc == 'r') {
            state = 507;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 507) { // "procedur"
        buffer += c;
        if (lc == 'e') {
            state = 508; // "procedure"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 508) { // "procedure"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 509; // Done
        return false;
    }
    
    if (state == 510) { // "prog"
        buffer += c;
        if (lc == 'r') {
            state = 511;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 511) { // "progr"
        buffer += c;
        if (lc == 'a') {
            state = 512;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 512) { // "progra"
        buffer += c;
        if (lc == 'm') {
            state = 513; // "program"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 513) { // "program"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 514; // Done
        return false;
    }
    
    // ===== JALUR A: array (34) =====
    if (state == 600) {
        buffer += c;
        if (lc == 'r') {
            state = 601;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 601) { // "ar"
        buffer += c;
        if (lc == 'r') {
            state = 602;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 602) { // "arr"
        buffer += c;
        if (lc == 'a') {
            state = 603;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 603) { // "arra"
        buffer += c;
        if (lc == 'y') {
            state = 604; // "array"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 604) { // "array"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 605; // Done
        return false;
    }
    
    // ===== JALUR R: record (35), repeat (41) =====
    if (state == 700) {
        buffer += c;
        if (lc == 'e') {
            state = 701;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 701) { // "re"
        buffer += c;
        if (lc == 'c') {
            state = 702;
            return true;
        } else if (lc == 'p') {
            state = 710;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 702) { // "rec"
        buffer += c;
        if (lc == 'o') {
            state = 703;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 703) { // "reco"
        buffer += c;
        if (lc == 'r') {
            state = 704;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 704) { // "recor"
        buffer += c;
        if (lc == 'd') {
            state = 705; // "record"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 705) { // "record"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 706;
        return false;
    }
    
    if (state == 710) { // "rep"
        buffer += c;
        if (lc == 'e') {
            state = 711;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 711) { // "repe"
        buffer += c;
        if (lc == 'a') {
            state = 712;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 712) { // "repea"
        buffer += c;
        if (lc == 't') {
            state = 713; // "repeat"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 713) { // "repeat"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 714; // Done
        return false;
    }
    
    // ===== JALUR B: begin (38) =====
    if (state == 800) {
        buffer += c;
        if (lc == 'e') {
            state = 801;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 801) { // "be"
        buffer += c;
        if (lc == 'g') {
            state = 802;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 802) { // "beg"
        buffer += c;
        if (lc == 'i') {
            state = 803;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 803) { // "begi"
        buffer += c;
        if (lc == 'n') {
            state = 804; // "begin"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 804) { // "begin"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 805;
        return false;
    }
    
    // ===== JALUR I: if (39) =====
    if (state == 900) {
        buffer += c;
        if (lc == 'f') {
            state = 901; // "if"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 901) { // "if"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 902;
        return false;
    }
    
    // ===== JALUR W: while (42) =====
    if (state == 1000) {
        buffer += c;
        if (lc == 'h') {
            state = 1001;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1001) { // "wh"
        buffer += c;
        if (lc == 'i') {
            state = 1002;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1002) { // "whi"
        buffer += c;
        if (lc == 'l') {
            state = 1003;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1003) { // "whil"
        buffer += c;
        if (lc == 'e') {
            state = 1004; // "while"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1004) { // "while"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 1005; // Done
        return false;
    }
    
    // ===== JALUR E: end (44), else (45) =====
    if (state == 1100) {
        buffer += c;
        if (lc == 'n') {
            state = 1101;
            return true;
        } else if (lc == 'l') {
            state = 1110;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1101) { // "en"
        buffer += c;
        if (lc == 'd') {
            state = 1102; // "end"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1102) { // "end"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 1103;
        return false;
    }
    
    if (state == 1110) { // "el"
        buffer += c;
        if (lc == 's') {
            state = 1111;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1111) { // "els"
        buffer += c;
        if (lc == 'e') {
            state = 1112; // "else"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1112) { // "else"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 1113;
        return false;
    }
    
    // ===== JALUR U: until (46) =====
    if (state == 1200) {
        buffer += c;
        if (lc == 'n') {
            state = 1201;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1201) { // "un"
        buffer += c;
        if (lc == 't') {
            state = 1202;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1202) { // "unt"
        buffer += c;
        if (lc == 'i') {
            state = 1203;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1203) { // "unti"
        buffer += c;
        if (lc == 'l') {
            state = 1204; // "until"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1204) { // "until"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 1205;
        return false;
    }
    
    // ===== JALUR O: of (47) =====
    if (state == 1300) {
        buffer += c;
        if (lc == 'f') {
            state = 1301; // "of"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1301) { // "of"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 1302;
        return false;
    }
    
    // ===== JALUR D: do (48), downto (50) =====
    if (state == 1400) {
        buffer += c;
        if (lc == 'o') {
            state = 1401; // "do"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1401) { // "do"
        buffer += c;
        if (lc == 'w') {
            state = 1402;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        state = 1410;
        return false;
    }
    
    if (state == 1402) { // "dow"
        buffer += c;
        if (lc == 'n') {
            state = 1403;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1403) { // "down"
        buffer += c;
        if (lc == 't') {
            state = 1404;
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1404) { // "downt"
        buffer += c;
        if (lc == 'o') {
            state = 1405; // "downto"
            return true;
        } else if (isLetter(c) || isDigit(c)) {
            state = 9999;
            return true;
        }
        return false;
    }
    
    if (state == 1405) { // "downto"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 1406; // Done
        return false;
    }
    
    if (state == 1410) { // "do"
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            state = 9999;
            return true;
        }
        state = 1411; // Done
        return false;
    }
    
    // ===== IDENTIFIER STATE (token 37) =====
    if (state == 9999) {
        if (isLetter(c) || isDigit(c)) {
            buffer += c;
            return true;
        }
        state = 10000;
        return false;
    }
    
    return false;
}

std::string KeywordIdentifierLexer::getToken() {
    if (buffer.empty()) {
        return "";
    }
    
    if (state == 105) {
        return "constsy"; // Token 29
    } else if (state == 113) {
        return "casesy"; // Token 40
    } else if (state == 204) {
        return "typesy"; // Token 30
    } else if (state == 213) {
        return "thensy"; // Token 51
    } else if (state == 221) {
        return "tosy"; // Token 49
    } else if (state == 303) {
        return "varsy"; // Token 31
    } else if (state == 408) {
        return "functionsy"; // Token 32
    } else if (state == 412) {
        return "forsy"; // Token 43
    } else if (state == 509) {
        return "proceduresy"; // Token 33
    } else if (state == 514) {
        return "programsy"; // Token 36
    } else if (state == 605) {
        return "arraysy"; // Token 34
    } else if (state == 706) {
        return "recordsy"; // Token 35
    } else if (state == 714) {
        return "repeatsy"; // Token 41
    } else if (state == 805) {
        return "beginsy"; // Token 38
    } else if (state == 902) {
        return "ifsy"; // Token 39
    } else if (state == 1005) {
        return "whilesy"; // Token 42
    } else if (state == 1103) {
        return "endsy"; // Token 44
    } else if (state == 1113) {
        return "elsesy"; // Token 45
    } else if (state == 1205) {
        return "untilsy"; // Token 46
    } else if (state == 1302) {
        return "ofsy"; // Token 47
    } else if (state == 1411) {
        return "dosy"; // Token 48
    } else if (state == 1406) {
        return "downtosy"; // Token 50
    } else if (state == 10000) {
        return "ident (" + buffer + ")"; // Token 37 - Identifier
    } else {

        std::string lowerBuf = "";
        for (size_t i = 0; i < buffer.length(); i++) {
            lowerBuf += toLower(buffer[i]);
        }
        
        // keyword
        if (lowerBuf == "const") return "constsy";
        if (lowerBuf == "case") return "casesy";
        if (lowerBuf == "type") return "typesy";
        if (lowerBuf == "then") return "thensy";
        if (lowerBuf == "to") return "tosy";
        if (lowerBuf == "var") return "varsy";
        if (lowerBuf == "function") return "functionsy";
        if (lowerBuf == "for") return "forsy";
        if (lowerBuf == "procedure") return "proceduresy";
        if (lowerBuf == "program") return "programsy";
        if (lowerBuf == "array") return "arraysy";
        if (lowerBuf == "record") return "recordsy";
        if (lowerBuf == "repeat") return "repeatsy";
        if (lowerBuf == "begin") return "beginsy";
        if (lowerBuf == "if") return "ifsy";
        if (lowerBuf == "while") return "whilesy";
        if (lowerBuf == "end") return "endsy";
        if (lowerBuf == "else") return "elsesy";
        if (lowerBuf == "until") return "untilsy";
        if (lowerBuf == "of") return "ofsy";
        if (lowerBuf == "do") return "dosy";
        if (lowerBuf == "downto") return "downtosy";
        
        // Default ke identifier
        return "ident (" + buffer + ")";
    }
}

bool KeywordIdentifierLexer::canStart(char c) {
    return isLetter(c);
}

std::string KeywordIdentifierLexer::getBuffer() {
    return buffer;
}

bool KeywordIdentifierLexer::isDone() {
    return (state >= 100 && state != 9999 && state != 0);
}
