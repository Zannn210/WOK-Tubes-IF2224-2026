#ifndef SEMANTICANALYZER_HPP
#define SEMANTICANALYZER_HPP

#include "ASTNode.hpp"
#include "SymbolTable.hpp"
#include <vector>
#include <string>
#include <fstream>

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    void analyze(ASTNode* root, const std::string& outFilename);

    const std::vector<TabEntry>&  getTab()  const { return tab; }
    const std::vector<BtabEntry>& getBtab() const { return btab; }
    const std::vector<AtabEntry>& getAtab() const { return atab; }
    bool hasSemanticError() const { return hasError; }

private:
    // Symbol tables
    std::vector<TabEntry> tab;
    std::vector<BtabEntry> btab;
    std::vector<AtabEntry> atab;
    std::vector<RangeEntry> rtab;

    std::vector<int> display;
    int currentLevel;
    int mainBlockIndex;
    int currentLine;

    std::ofstream outFile;
    bool hasError;

    // Initialisation 
    void initPredefined();

    // Scope / symbol management
    int ensureMainBlock();
    int enterBlock(); 
    void leaveBlock();
    int addIdentifier(const std::string& name, int obj, int type, int ref, int nrm, int adr);
    int lookupIdent(const std::string& name, bool errorIfMissing = true);
    int resolveTypeName(const std::string& name);
    int getTypeSize(int typeCode, int ref) const;

    // Visitor functions
    void visitProgram(ASTNode* node);
    void visitDeclarationPart(ASTNode* node);
    void visitConstDeclaration(ASTNode* node);
    int visitConstant(ASTNode* node); 
    void visitTypeDeclaration(ASTNode* node);
    int visitType(ASTNode* node, int& outRef);  
    int visitArrayType(ASTNode* node, int& outRef);
    int visitRange(ASTNode* node, int& outRef, int* low = nullptr, int* high = nullptr);
    int visitEnumerated(ASTNode* node, int& outRef);
    int visitRecordType(ASTNode* node, int& outRef);
    void visitFieldList(ASTNode* node);
    void visitFieldPart(ASTNode* node);
    void visitVarDeclaration(ASTNode* node);
    void visitSubprogramDeclaration(ASTNode* node);
    void visitProcedureDeclaration(ASTNode* node);
    void visitFunctionDeclaration(ASTNode* node);
    void visitFormalParameterList(ASTNode* node);
    void visitParameterGroup(ASTNode* node);
    void visitBlock(ASTNode* node);
    void visitCompoundStatement(ASTNode* node);
    void visitStatementList(ASTNode* node);
    void visitStatement(ASTNode* node);
    void visitAssignmentStatement(ASTNode* node);
    void visitIfStatement(ASTNode* node);
    void visitCaseStatement(ASTNode* node);
    void visitCaseBlock(ASTNode* node);
    void visitWhileStatement(ASTNode* node);
    void visitRepeatStatement(ASTNode* node);
    void visitForStatement(ASTNode* node);
    void visitProcFuncCall(ASTNode* node);
    int visitExpression(ASTNode* node); 
    int visitSimpleExpression(ASTNode* node);
    int visitTerm(ASTNode* node);
    int visitFactor(ASTNode* node);
    int visitVariable(ASTNode* node);
    int visitComponentVariable(ASTNode* node,  int parentType, int parentRef, int& newRef);
    void visitIndexList(ASTNode* node);

    // Helper semantic untuk validasi pemanggilan subprogram dan index array.
    std::vector<int> getFormalParameterIndices(int calleeIdx) const;
    std::vector<ASTNode*> collectActualArguments(ASTNode* paramList) const;
    std::vector<ASTNode*> collectIndexNodes(ASTNode* indexList) const;
    int inferIndexNodeType(ASTNode* node);

    // Type helpers
    bool typesCompatible(int t1, int t2) const;
    bool assignCompatible(int targetType, int valueType) const;
    bool isRelOp(const std::string& tokenType) const;
    bool isNumeric(int typeCode) const;

    // Line and constant helpers
    int  getNodeLine(ASTNode* node) const;
    bool tryGetConstInt(ASTNode* node, int& val) const;

    void emit(const std::string& s);

    std::string typeCodeName(int t) const;
    std::string objKindName(int o)  const;
    std::string toLower(const std::string& s) const;

    // Error
    void semanticError(const std::string& msg);
    void semanticWarning(const std::string& msg);
};

#endif 
