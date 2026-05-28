#ifndef STACK_INTERPRETER_HPP
#define STACK_INTERPRETER_HPP

#include "IntermediateCode.hpp"
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

struct RuntimeValue {
    // Representasi nilai runtime yang dapat menyimpan berbagai tipe data (integer, real, boolean, char, string, address)
    enum Kind { NONE, INTEGER, REAL, BOOLEAN, CHAR, STRING, ADDRESS } kind = NONE;
    long long i = 0;
    double r = 0.0;
    bool b = false;
    char c = '\0';
    std::string s;

    // Factory methods untuk membuat nilai runtime dari tipe dasar
    static RuntimeValue none();
    static RuntimeValue integer(long long v);
    static RuntimeValue real(double v);
    static RuntimeValue boolean(bool v);
    static RuntimeValue character(char v);
    static RuntimeValue string(std::string v);
    static RuntimeValue address(int blockId, int offset);

    bool truthy() const;
    std::string toString() const;
    int addressBlock() const;
    int addressOffset() const;
};

// Interpreter stack machine yang mengeksekusi intermediate code
class StackInterpreter {
public:
    explicit StackInterpreter(Code program, std::size_t stackLimit = 8192);
    void run(std::ostream& out);

private:
    // Frame aktivasi untuk subprogram
    struct Frame {
        int blockId = 0;
        std::vector<RuntimeValue> mem;
        int returnPc = 0;
        bool hasReturnValue = false;
    };

    // Data internal interpreter
    Code program;
    std::vector<RuntimeValue> globalMemory;
    std::vector<Frame> frames;
    std::vector<RuntimeValue> operandStack;
    std::size_t stackLimit;
    int pc = 0;
    bool halted = false;

    // Helper parsing dan stack manipulation
    RuntimeValue parseLiteral(const std::string& raw) const;
    void push(const RuntimeValue& v);
    RuntimeValue pop();
    RuntimeValue peek() const;
    RuntimeValue& resolveCell(int blockId, int offset);
    const RuntimeValue& resolveCell(int blockId, int offset) const;
    RuntimeValue& resolveAddress(const RuntimeValue& addr);
    const RuntimeValue& resolveAddress(const RuntimeValue& addr) const;
    void validateJump(int target) const;

    // Operasi aritmatika dan perbandingan untuk eksekusi OPR
    RuntimeValue numericOp(const RuntimeValue& a, const RuntimeValue& b, int opr) const;
    RuntimeValue compareOp(const RuntimeValue& a, const RuntimeValue& b, int opr) const;

    void execOpr(int opr, std::ostream& out);
    void execCal(const Instruction& ins);
};

#endif
