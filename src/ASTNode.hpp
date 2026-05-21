#ifndef ASTNODE_HPP
#define ASTNODE_HPP

#include <string>
#include <vector>

enum TypeCode {
    TYPE_VOID  = 0,
    TYPE_INTEGER = 1,
    TYPE_REAL = 2,
    TYPE_BOOLEAN =  3,
    TYPE_CHAR = 4,
    TYPE_STRING =  5,
    TYPE_ARRAY = 6,
    TYPE_RECORD= 7,
    TYPE_SUBRANGE = 8,
    TYPE_ENUMERATED = 9,
    TYPE_UNKNOWN = -1
};


struct ASTNode {
    std::string label; 
    bool isTerminal; 
    std::string tokenType;  
    std::string tokenValue;  

    std::vector<ASTNode*> children;

    int semType; 
    int tabIdx; 
    int lexLevel; 

    ASTNode(const std::string& lbl,bool term = false,const std::string& tt = "", const std::string& tv = "")
        : label(lbl), isTerminal(term),tokenType(tt), tokenValue(tv), semType(TYPE_VOID), tabIdx(-1), lexLevel(0) {}

    ~ASTNode() {
        for (auto* c : children) delete c;
    }

    void addChild(ASTNode* c) {
        if (c) children.push_back(c);
    }
};

#endif
