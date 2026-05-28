#include "StackInterpreter.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
// Basis pengkodean alamat: blockId * ADDR_BASE + offset
constexpr long long ADDR_BASE = 1000000LL;

// Cek apakah nilai runtime adalah tipe numerik (integer, real, boolean, char)
bool isNumeric(const RuntimeValue& v) {
    return v.kind == RuntimeValue::INTEGER || v.kind == RuntimeValue::REAL ||
           v.kind == RuntimeValue::BOOLEAN || v.kind == RuntimeValue::CHAR;
}

// Konversi nilai runtime ke double (untuk operasi aritmatika)
double asReal(const RuntimeValue& v) {
    if (v.kind == RuntimeValue::REAL) return v.r;
    if (v.kind == RuntimeValue::INTEGER || v.kind == RuntimeValue::BOOLEAN ||
        v.kind == RuntimeValue::CHAR || v.kind == RuntimeValue::ADDRESS) return static_cast<double>(v.i);
    return 0.0;
}

// Konversi nilai runtime ke integer (untuk operasi integer dan indeks)
long long asInt(const RuntimeValue& v) {
    if (v.kind == RuntimeValue::REAL) return static_cast<long long>(v.r);
    if (v.kind == RuntimeValue::CHAR) return static_cast<unsigned char>(v.c);
    if (v.kind == RuntimeValue::BOOLEAN) return v.b ? 1 : 0;
    return v.i;
}
}

// Factory methods untuk membuat nilai runtime
RuntimeValue RuntimeValue::none() { return {}; }
RuntimeValue RuntimeValue::integer(long long v) { RuntimeValue x; x.kind = INTEGER; x.i = v; x.r = static_cast<double>(v); x.b = v != 0; return x; }
RuntimeValue RuntimeValue::real(double v) { RuntimeValue x; x.kind = REAL; x.r = v; x.i = static_cast<long long>(v); x.b = std::fabs(v) > 1e-12; return x; }
RuntimeValue RuntimeValue::boolean(bool v) { RuntimeValue x; x.kind = BOOLEAN; x.b = v; x.i = v ? 1 : 0; x.r = v ? 1.0 : 0.0; return x; }
RuntimeValue RuntimeValue::character(char v) { RuntimeValue x; x.kind = CHAR; x.c = v; x.i = static_cast<unsigned char>(v); x.r = static_cast<double>(x.i); x.b = v != '\0'; return x; }
RuntimeValue RuntimeValue::string(std::string v) { RuntimeValue x; x.kind = STRING; x.s = std::move(v); x.b = !x.s.empty(); return x; }
RuntimeValue RuntimeValue::address(int blockId, int offset) { RuntimeValue x; x.kind = ADDRESS; x.i = blockId * ADDR_BASE + offset; x.r = static_cast<double>(x.i); return x; }

// Evaluasi kebenaran nilai (untuk kondisi if, while, dll)
bool RuntimeValue::truthy() const {
    switch (kind) {
        case BOOLEAN: return b;
        case INTEGER: return i != 0;
        case REAL: return std::fabs(r) > 1e-12;
        case CHAR: return c != '\0';
        case STRING: return !s.empty();
        case ADDRESS: return true;
        default: return false;
    }
}

// Konversi nilai ke string untuk output (writeln)
std::string RuntimeValue::toString() const {
    std::ostringstream oss;
    switch (kind) {
        case INTEGER: oss << i; break;
        case REAL: oss << std::setprecision(12) << r; break;
        case BOOLEAN: oss << (b ? "true" : "false"); break;
        case CHAR: oss << c; break;
        case STRING: oss << s; break;
        case ADDRESS: oss << "&(" << addressBlock() << ":" << addressOffset() << ")"; break;
        default: break;
    }
    return oss.str();
}

// Ekstrak blockId dari nilai alamat
int RuntimeValue::addressBlock() const {
    if (kind != ADDRESS) throw std::runtime_error("Runtime Error: value is not an address");
    return static_cast<int>(i / ADDR_BASE);
}

// Ekstrak offset dari nilai alamat
int RuntimeValue::addressOffset() const {
    if (kind != ADDRESS) throw std::runtime_error("Runtime Error: value is not an address");
    return static_cast<int>(i % ADDR_BASE);
}

// Constructor: simpan program dan batas maksimum stack
StackInterpreter::StackInterpreter(Code p, std::size_t limit)
    : program(std::move(p)), stackLimit(limit) {}

// Parsing literal dari string (integer, real, string, char, boolean)
RuntimeValue StackInterpreter::parseLiteral(const std::string& raw) const {
    if (raw.empty()) return RuntimeValue::integer(0);
    if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
        return RuntimeValue::string(raw.substr(1, raw.size() - 2));
    }
    if (raw.size() >= 3 && raw.front() == '\'' && raw.back() == '\'') {
        return RuntimeValue::character(raw[1]);
    }
    if (raw == "true") return RuntimeValue::boolean(true);
    if (raw == "false") return RuntimeValue::boolean(false);
    if (raw.find('.') != std::string::npos) return RuntimeValue::real(std::stod(raw));
    return RuntimeValue::integer(std::stoll(raw));
}

// Operasi stack dasar
void StackInterpreter::push(const RuntimeValue& v) {
    if (operandStack.size() >= stackLimit) throw std::runtime_error("Runtime Error: Stack Overflow");
    operandStack.push_back(v);
}

RuntimeValue StackInterpreter::pop() {
    if (operandStack.empty()) throw std::runtime_error("Runtime Error: Stack Underflow");
    RuntimeValue v = operandStack.back();
    operandStack.pop_back();
    return v;
}

RuntimeValue StackInterpreter::peek() const {
    if (operandStack.empty()) throw std::runtime_error("Runtime Error: Stack Underflow");
    return operandStack.back();
}

// Dapatkan referensi ke sel memori (global atau frame) berdasarkan blockId dan offset
RuntimeValue& StackInterpreter::resolveCell(int blockId, int offset) {
    if (offset < 0) throw std::runtime_error("Runtime Error: Memory Access Out of Bounds");
    if (blockId == 0) {
        if (offset >= static_cast<int>(globalMemory.size()))
            throw std::runtime_error("Runtime Error: Memory Access Out of Bounds at global address " + std::to_string(offset));
        return globalMemory[offset];
    }
    for (auto it = frames.rbegin(); it != frames.rend(); ++it) {
        if (it->blockId == blockId) {
            if (offset >= static_cast<int>(it->mem.size()))
                throw std::runtime_error("Runtime Error: Memory Access Out of Bounds at frame " + std::to_string(blockId) + ":" + std::to_string(offset));
            return it->mem[offset];
        }
    }
    throw std::runtime_error("Runtime Error: Activation frame not found for block " + std::to_string(blockId));
}

const RuntimeValue& StackInterpreter::resolveCell(int blockId, int offset) const {
    return const_cast<StackInterpreter*>(this)->resolveCell(blockId, offset);
}

// Dapatkan referensi ke sel memori dari nilai alamat
RuntimeValue& StackInterpreter::resolveAddress(const RuntimeValue& addr) {
    return resolveCell(addr.addressBlock(), addr.addressOffset());
}

const RuntimeValue& StackInterpreter::resolveAddress(const RuntimeValue& addr) const {
    return resolveCell(addr.addressBlock(), addr.addressOffset());
}

// Validasi target lompatan (pastikan dalam rentang program)
void StackInterpreter::validateJump(int target) const {
    if (target < 0 || target >= static_cast<int>(program.size()))
        throw std::runtime_error("Runtime Error: Label not found / invalid jump target " + std::to_string(target));
}

// Operasi aritmatika (+, -, *, /, mod) dengan dukungan address arithmetic dan string concatenation
RuntimeValue StackInterpreter::numericOp(const RuntimeValue& a, const RuntimeValue& b, int opr) const {
    if (a.kind == RuntimeValue::ADDRESS && b.kind == RuntimeValue::INTEGER && opr == OPR_ADD)
        return RuntimeValue::address(a.addressBlock(), a.addressOffset() + static_cast<int>(b.i));
    if (b.kind == RuntimeValue::ADDRESS && a.kind == RuntimeValue::INTEGER && opr == OPR_ADD)
        return RuntimeValue::address(b.addressBlock(), b.addressOffset() + static_cast<int>(a.i));
    if (a.kind == RuntimeValue::ADDRESS && b.kind == RuntimeValue::INTEGER && opr == OPR_SUB)
        return RuntimeValue::address(a.addressBlock(), a.addressOffset() - static_cast<int>(b.i));

    if ((a.kind == RuntimeValue::STRING || b.kind == RuntimeValue::STRING) && opr == OPR_ADD)
        return RuntimeValue::string(a.toString() + b.toString());

    const bool realResult = a.kind == RuntimeValue::REAL || b.kind == RuntimeValue::REAL || opr == OPR_DIV;
    const double ar = asReal(a), br = asReal(b);
    const long long ai = asInt(a), bi = asInt(b);

    switch (opr) {
        case OPR_ADD: return realResult ? RuntimeValue::real(ar + br) : RuntimeValue::integer(ai + bi);
        case OPR_SUB: return realResult ? RuntimeValue::real(ar - br) : RuntimeValue::integer(ai - bi);
        case OPR_MUL: return realResult ? RuntimeValue::real(ar * br) : RuntimeValue::integer(ai * bi);
        case OPR_DIV:
            if (std::fabs(br) < 1e-12) throw std::runtime_error("Runtime Error: Division by zero");
            if (a.kind == RuntimeValue::INTEGER && b.kind == RuntimeValue::INTEGER) return RuntimeValue::integer(ai / bi);
            return RuntimeValue::real(ar / br);
        case OPR_MOD:
            if (bi == 0) throw std::runtime_error("Runtime Error: Modulo by zero");
            return RuntimeValue::integer(ai % bi);
    }
    return RuntimeValue::none();
}

// Operasi perbandingan (==, !=, <, <=, >, >=) untuk nilai numerik dan string
RuntimeValue StackInterpreter::compareOp(const RuntimeValue& a, const RuntimeValue& b, int opr) const {
    if (a.kind == RuntimeValue::STRING || b.kind == RuntimeValue::STRING) {
        const std::string as = a.toString();
        const std::string bs = b.toString();
        switch (opr) {
            case OPR_EQL: return RuntimeValue::boolean(as == bs);
            case OPR_NEQ: return RuntimeValue::boolean(as != bs);
            case OPR_LSS: return RuntimeValue::boolean(as < bs);
            case OPR_LEQ: return RuntimeValue::boolean(as <= bs);
            case OPR_GTR: return RuntimeValue::boolean(as > bs);
            case OPR_GEQ: return RuntimeValue::boolean(as >= bs);
        }
    }
    const double ar = asReal(a), br = asReal(b);
    switch (opr) {
        case OPR_EQL: return RuntimeValue::boolean(std::fabs(ar - br) < 1e-12);
        case OPR_NEQ: return RuntimeValue::boolean(std::fabs(ar - br) >= 1e-12);
        case OPR_LSS: return RuntimeValue::boolean(ar < br);
        case OPR_LEQ: return RuntimeValue::boolean(ar <= br);
        case OPR_GTR: return RuntimeValue::boolean(ar > br);
        case OPR_GEQ: return RuntimeValue::boolean(ar >= br);
    }
    return RuntimeValue::boolean(false);
}

// Eksekusi instruksi OPR berdasarkan kode operasi (dispatcher)
void StackInterpreter::execOpr(int opr, std::ostream& out) {
    if (opr == OPR_NOOP) return;
    if (opr == OPR_NEG) {
        RuntimeValue v = pop();
        if (v.kind == RuntimeValue::REAL) push(RuntimeValue::real(-v.r));
        else push(RuntimeValue::integer(-asInt(v)));
        return;
    }
    if (opr == OPR_NOT) {
        push(RuntimeValue::boolean(!pop().truthy()));
        return;
    }
    if (opr == OPR_WRITE) {
        out << pop().toString();
        return;
    }
    if (opr == OPR_NEWLINE) {
        out << '\n';
        return;
    }
    if (opr == OPR_READLN) {
        std::string s;
        std::cin >> s;
        push(parseLiteral(s));
        return;
    }
    if (opr == OPR_TO_CHAR) {
        RuntimeValue v = pop();
        push(RuntimeValue::character(static_cast<char>(asInt(v))));
        return;
    }

    RuntimeValue right = pop();
    RuntimeValue left = pop();
    if (opr >= OPR_ADD && opr <= OPR_MOD) { push(numericOp(left, right, opr)); return; }
    if (opr >= OPR_EQL && opr <= OPR_GEQ) { push(compareOp(left, right, opr)); return; }
    if (opr == OPR_AND) { push(RuntimeValue::boolean(left.truthy() && right.truthy())); return; }
    if (opr == OPR_OR) { push(RuntimeValue::boolean(left.truthy() || right.truthy())); return; }
    throw std::runtime_error("Runtime Error: Unknown OPR " + std::to_string(opr));
}

// Eksekusi instruksi CAL: siapkan frame baru, salin parameter, lompat ke subprogram
void StackInterpreter::execCal(const Instruction& ins) {
    validateJump(ins.arg);
    // literal format produced by the generator: "params=<n>;size=<s>;ret=<0|1>"
    int params = 0, size = 0, ret = 0;
    std::stringstream ss(ins.literal);
    std::string part;
    while (std::getline(ss, part, ';')) {
        auto pos = part.find('=');
        if (pos == std::string::npos) continue;
        std::string k = part.substr(0, pos);
        int v = std::stoi(part.substr(pos + 1));
        if (k == "params") params = v;
        else if (k == "size") size = v;
        else if (k == "ret") ret = v;
    }
    int firstParamOffset = ret ? 1 : 0;
    if (size < params + firstParamOffset) size = params + firstParamOffset;

    Frame f;
    f.blockId = ins.level;
    f.mem.assign(size, RuntimeValue::none());
    f.returnPc = pc;
    f.hasReturnValue = (ret != 0);

    for (int i = params - 1; i >= 0; --i) {
        RuntimeValue actual = pop();
        f.mem[firstParamOffset + i] = actual;
    }
    frames.push_back(std::move(f));
    pc = ins.arg;
}

// Eksekusi program: fetch-decode-execute cycle
void StackInterpreter::run(std::ostream& out) {
    pc = 0;
    halted = false;
    globalMemory.clear();
    frames.clear();
    operandStack.clear();

    while (!halted && pc >= 0 && pc < static_cast<int>(program.size())) {
        const Instruction& ins = program[pc++];

        if (ins.op == "INT") {
            if (ins.arg < 0) throw std::runtime_error("Runtime Error: negative memory size");
            if (ins.level == 0) globalMemory.assign(static_cast<std::size_t>(ins.arg), RuntimeValue::none());
            else if (!frames.empty() && frames.back().blockId == ins.level && static_cast<int>(frames.back().mem.size()) < ins.arg)
                frames.back().mem.resize(ins.arg, RuntimeValue::none());
        } else if (ins.op == "LIT") {
            push(parseLiteral(!ins.literal.empty() ? ins.literal : std::to_string(ins.arg)));
        } else if (ins.op == "LOD") {
            push(resolveCell(ins.level, ins.arg));
        } else if (ins.op == "STO") {
            resolveCell(ins.level, ins.arg) = pop();
        } else if (ins.op == "LDA") {
            push(RuntimeValue::address(ins.level, ins.arg));
        } else if (ins.op == "LDI") {
            RuntimeValue addr = pop();
            push(resolveAddress(addr));
        } else if (ins.op == "STI") {
            RuntimeValue val = pop();
            RuntimeValue addr = pop();
            resolveAddress(addr) = val;
        } else if (ins.op == "CHK") {
            RuntimeValue idx = peek();
            long long v = asInt(idx);
            if (v < ins.level || v > ins.arg) {
                throw std::runtime_error("Runtime Error: Array Index Out of Bounds: " + std::to_string(v) +
                                         " not in [" + std::to_string(ins.level) + ".." + std::to_string(ins.arg) + "]");
            }
        } else if (ins.op == "POP") {
            (void)pop();
        } else if (ins.op == "JMP") {
            validateJump(ins.arg);
            pc = ins.arg;
        } else if (ins.op == "JPC") {
            RuntimeValue cond = pop();
            if (!cond.truthy()) { validateJump(ins.arg); pc = ins.arg; }
        } else if (ins.op == "CAL") {
            execCal(ins);
        } else if (ins.op == "RET") {
            if (frames.empty()) {
                halted = true;
            } else {
                Frame f = frames.back();
                frames.pop_back();
                if (f.hasReturnValue) push(f.mem.empty() ? RuntimeValue::none() : f.mem[0]);
                pc = f.returnPc;
            }
        } else if (ins.op == "OPR") {
            execOpr(ins.arg, out);
        } else {
            throw std::runtime_error("Runtime Error: Unknown instruction " + ins.op);
        }
    }

    if (!halted && pc != static_cast<int>(program.size()))
        throw std::runtime_error("Runtime Error: PC out of bounds");
}
