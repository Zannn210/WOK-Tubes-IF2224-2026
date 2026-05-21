#ifndef ASTNODE_HPP
#define ASTNODE_HPP

#include <string>
#include <vector>

// Semantic type codes used throughout the compiler
enum TypeCode {
    TYPE_VOID       =  0,
    TYPE_INTEGER    =  1,
    TYPE_REAL       =  2,
    TYPE_BOOLEAN    =  3,
    TYPE_CHAR       =  4,
    TYPE_STRING     =  5,
    TYPE_ARRAY      =  6,
    TYPE_RECORD     =  7,
    TYPE_SUBRANGE   =  8,
    TYPE_ENUMERATED =  9,
    TYPE_UNKNOWN    = -1
};

// A single node in the parse tree / AST.
// Built by the parser; annotated by the semantic analyzer.
struct ASTNode {
    std::string label;       // e.g. "<program>", "ident(Hello)", "plus"
    bool        isTerminal;  // true for token leaf nodes
    std::string tokenType;   // e.g. "ident", "intcon", "plus"
    std::string tokenValue;  // e.g. "Hello", "5", ""

    std::vector<ASTNode*> children;

    // Semantic annotations (filled by SemanticAnalyzer)
    int semType;    // TypeCode of this expression/node
    int tabIdx;     // index into tab (-1 if not applicable)
    int lexLevel;   // lexical level where this node was declared/used

    ASTNode(const std::string& lbl,
            bool term             = false,
            const std::string& tt = "",
            const std::string& tv = "")
        : label(lbl), isTerminal(term),
          tokenType(tt), tokenValue(tv),
          semType(TYPE_VOID), tabIdx(-1), lexLevel(0) {}

    ~ASTNode() {
        for (auto* c : children) delete c;
    }

    void addChild(ASTNode* c) {
        if (c) children.push_back(c);
    }
};

#endif // ASTNODE_HPP
