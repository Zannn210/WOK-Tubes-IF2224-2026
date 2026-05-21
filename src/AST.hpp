#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory>

// Kelas dasar untuk semua node AST
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// Kelas turunan utama
class ExprNode : public ASTNode {};
class StatementNode : public ASTNode {};
class DeclNode : public ASTNode {};

// --- IMPLEMENTASI NODE EKSPRESI ---

class NumberNode : public ExprNode {
public:
    std::string value;
    std::string type; // "intcon" atau "realcon"
    NumberNode(const std::string& v, const std::string& t) : value(v), type(t) {}
};

class StringNode : public ExprNode {
public:
    std::string value;
    StringNode(const std::string& v) : value(v) {}
};

class VarNode : public ExprNode {
public:
    std::string name;
    VarNode(const std::string& n) : name(n) {}
};

class BinOpNode : public ExprNode {
public:
    std::string op;
    std::shared_ptr<ExprNode> left;
    std::shared_ptr<ExprNode> right;
    BinOpNode(const std::string& o, std::shared_ptr<ExprNode> l, std::shared_ptr<ExprNode> r)
        : op(o), left(l), right(r) {}
};

// --- IMPLEMENTASI NODE STATEMENT ---

class AssignNode : public StatementNode {
public:
    std::shared_ptr<VarNode> target; // Variabel tujuan
    std::shared_ptr<ExprNode> value; // Nilai yang di-assign
    AssignNode(std::shared_ptr<VarNode> t, std::shared_ptr<ExprNode> v)
        : target(t), value(v) {}
};

class ProcCallNode : public StatementNode {
public:
    std::string procName;
    std::vector<std::shared_ptr<ExprNode>> args;
    ProcCallNode(const std::string& name, const std::vector<std::shared_ptr<ExprNode>>& a)
        : procName(name), args(a) {}
};

class CompoundStmtNode : public StatementNode {
public:
    std::vector<std::shared_ptr<StatementNode>> statements;
    CompoundStmtNode(const std::vector<std::shared_ptr<StatementNode>>& stmts)
        : statements(stmts) {}
};

// (Bisa dilanjutkan untuk IfNode, WhileNode, ForNode sesuai kebutuhan parser)

#endif
