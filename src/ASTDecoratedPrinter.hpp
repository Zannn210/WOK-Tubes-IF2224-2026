#ifndef ASTDECORATEDPRINTER_HPP
#define ASTDECORATEDPRINTER_HPP

#include "ASTNode.hpp"
#include "SymbolTable.hpp"
#include <vector>
#include <string>
#include <fstream>

class ASTDecoratedPrinter {
public:
    ASTDecoratedPrinter(
        const std::vector<TabEntry>&  tab,
        const std::vector<BtabEntry>& btab,
        const std::vector<AtabEntry>& atab,
        std::ofstream& outFile
    );

    // Main entry
    void printAll(ASTNode* root);

private:
    const std::vector<TabEntry>&  _tab;
    const std::vector<BtabEntry>& _btab;
    const std::vector<AtabEntry>& _atab;
    std::ofstream& _out;

    // Emit
    void emit(const std::string& s);
    std::string ind(int d) const;   // d*2 spaces

    // Type /object name helpers
    std::string typeFull(int t)  const;  
    std::string typeShort(int t) const;  
    std::string objClass(int o)  const;   
    std::string opSym(const std::string& tok) const; 
    std::string inlineExpr(ASTNode* n) const;
    bool isRelOp(const std::string& tok) const;
    bool isAddOp(const std::string& tok) const;
    bool isMulOp(const std::string& tok) const;

    // Parse-tree navigation helpers 
    ASTNode* childNT(ASTNode* n, const std::string& label) const;
    ASTNode* childTerm(ASTNode* n, const std::string& tok) const;
    std::string progName(ASTNode* programNode) const;
    std::string stripQuotes(const std::string& s) const;

    // Decorated AST printers 
    void doProg(ASTNode* n, int d);
    void doDecls(ASTNode* n, int d);
    void doVarDecl(ASTNode* n, int d, bool isParam = false);
    void doConstDecl(ASTNode* n, int d);
    void doTypeDecl(ASTNode* n, int d);
    void doRecordFields(int blockIdx, int d);
    void doSubprog(ASTNode* n, int d);
    void doProcDecl(ASTNode* n, int d);
    void doFuncDecl(ASTNode* n, int d);
    void doBlock(ASTNode* n, int d);  
    void doCompound(ASTNode* n, int d);
    void doStmtList(ASTNode* n, int d);
    void doStmt(ASTNode* n, int d);
    void doAssign(ASTNode* n, int d);
    void doIf(ASTNode* n, int d);
    void doWhile(ASTNode* n, int d);
    void doFor(ASTNode* n, int d);
    void doRepeat(ASTNode* n, int d);
    void doCase(ASTNode* n, int d);
    void doCaseBlock(ASTNode* n, int d);
    void doProcCall(ASTNode* n, int d);


    void doExpr(ASTNode* n, int d);
    void doFactor(ASTNode* n, int d);
    void doVarRef(ASTNode* n, int d);   
    void doConstantNode(ASTNode* n, int d); 
    void doBinChain(  
        const std::vector<ASTNode*>& operands,
        const std::vector<std::string>& opToks,
        int resultType,
        int d);

    // Symbol-table printers 
    void doTab();
    void doBtab();
    void doAtab();
};

#endif 
