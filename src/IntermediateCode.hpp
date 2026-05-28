#ifndef INTERMEDIATE_CODE_HPP
#define INTERMEDIATE_CODE_HPP

#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Satu baris instruksi untuk VM stack machine
struct Instruction {
    std::string op;      // mnemonic instruksi: LIT, LOD, STO, OPR, JMP, dst
    int level = 0;       // untuk global/frame/procedure block. Untuk CHK, dipakai sebagai low bound
    int arg = 0;         // operand angka/alamat/target jump. Untuk CHK, dipakai sebagai high bound
    std::string literal; // operand non-angka, misalnya string literal atau metadata CAL
    std::string comment; // komentar yang ikut ditulis ke file IC setelah tanda ';'

    Instruction() = default;
    Instruction(std::string o, int l, int a, std::string lit = "", std::string c = "")
        : op(std::move(o)), level(l), arg(a), literal(std::move(lit)), comment(std::move(c)) {}

    // Ubah instruksi ke format output IC yang enak dibaca
    std::string toString(int index, bool withComment = true) const {
        std::ostringstream oss;
        oss << index << ' ' << op;
        if (op == "RET" || op == "LDI" || op == "STI" || op == "POP") {
            oss << " 0 0";
        } else if (op == "LIT" && !literal.empty()) {
            oss << ' ' << level << ' ' << literal;
        } else {
            oss << ' ' << level << ' ' << arg;
        }
        if (withComment && !comment.empty()) oss << " ; " << comment;
        return oss.str();
    }
};

// Kode operasi untuk instruksi OPR
// 1-14 mengikuti konvensi utama milestone
// 15-20 tambahan untuk boolean, newline, konversi char, dan no-op
enum OprCode {
    OPR_NEG = 1,
    OPR_ADD = 2,
    OPR_SUB = 3,
    OPR_MUL = 4,
    OPR_DIV = 5,
    OPR_MOD = 6,
    OPR_EQL = 7,
    OPR_NEQ = 8,
    OPR_LSS = 9,
    OPR_LEQ = 10,
    OPR_GTR = 11,
    OPR_GEQ = 12,
    OPR_READLN = 13,
    OPR_WRITE = 14,
    OPR_NOT = 15,
    OPR_AND = 16,
    OPR_OR = 17,
    OPR_NEWLINE = 18,
    OPR_TO_CHAR = 19,
    OPR_NOOP = 20
};

using Code = std::vector<Instruction>;

#endif
