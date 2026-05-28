#ifndef INTERMEDIATE_CODE_GENERATOR_HPP
#define INTERMEDIATE_CODE_GENERATOR_HPP

#include "ASTNode.hpp"
#include "IntermediateCode.hpp"
#include "SymbolTable.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

// Generator ini mengubah Decorated AST menjadi instruksi stack machine
class IntermediateCodeGenerator {
public:
    IntermediateCodeGenerator(const std::vector<TabEntry>& tab,
                              const std::vector<BtabEntry>& btab,
                              const std::vector<AtabEntry>& atab);

    Code generate(ASTNode* root, const std::string& outFilename = "");

private:
    // Lokasi runtime sebuah variabel di memori VM
    // blockId 0 berarti global, selain itu berarti frame procedure/function
    struct VarLocation {
        int blockId = 0;
        int offset = 0;
        int size = 1;
        int type = TYPE_UNKNOWN;
        int ref = 0;
    };

    // Metadata procedure/function yang dibutuhkan saat generate CAL dan RET
    struct SubprogramInfo {
        int tabIdx = -1;        // index symbol di tab
        int blockId = 0;        // id frame runtime
        int entry = -1;         // alamat instruksi pertama subprogram
        int frameSize = 0;      // jumlah slot memori lokal + parameter
        int paramCount = 0;     // jumlah parameter, berguna buat hitung offset frame lokal
        bool isFunction = false;
        ASTNode* declaration = nullptr;
        ASTNode* block = nullptr;
        std::string name;
    };

    const std::vector<TabEntry>& tab;
    const std::vector<BtabEntry>& btab;
    const std::vector<AtabEntry>& atab;
    Code code;

    std::map<int, VarLocation> locations;       // key: index tab variabel
    std::map<int, SubprogramInfo> subprograms;  // key: index tab procedure/function
    std::vector<int> subprogramOrder;
    int globalSize = 0;
    int currentBlockId = 0;
    int currentFunctionTabIdx = -1;

    // Emiter & backpatch
    int emit(const std::string& op, int level, int arg,
             const std::string& literal = "", const std::string& comment = "");
    void patchArg(int instructionIndex, int newArg);
    void writeToFile(const std::string& filename) const;

    // Helper AST
    static std::string lower(std::string s);
    static bool isTerminal(ASTNode* n, const std::string& tokenType);
    ASTNode* childNT(ASTNode* n, const std::string& label) const;
    ASTNode* childTerm(ASTNode* n, const std::string& tokenType) const;
    std::vector<ASTNode*> childrenNT(ASTNode* n, const std::string& label) const;
    std::vector<ASTNode*> expressions(ASTNode* n) const;

    // Metadata & symbol table
    int typeSize(int type, int ref) const;
    int symbolSize(int tabIdx) const;
    int findSymbol(const std::string& name, int objKind = -1) const;
    std::string stripQuotes(const std::string& s) const;
    std::string makeLiteral(ASTNode* terminal) const;
    int opFromToken(const std::string& tokenType) const;

    // Memory layouting
    void prepareLayouts(ASTNode* root);
    void collectGlobalVars(ASTNode* declPart);
    void collectSubprograms(ASTNode* declPart);
    void buildSubprogramLayout(SubprogramInfo& info);
    void addVarLocationFromIdent(ASTNode* ident, int blockId, int& nextOffset);
    int allocTemp();

    // Code gen
    void genProgram(ASTNode* node);
    void genSubprogram(const SubprogramInfo& info);
    void genCompound(ASTNode* node);
    void genStatementList(ASTNode* node);
    void genStatement(ASTNode* node);
    void genAssignment(ASTNode* node);
    void genIf(ASTNode* node);
    void genCase(ASTNode* node);
    void collectCaseBranches(ASTNode* caseBlock, std::vector<std::pair<std::vector<ASTNode*>, ASTNode*>>& branches) const;
    void genWhile(ASTNode* node);
    void genRepeat(ASTNode* node);
    void genFor(ASTNode* node);
    void genProcFuncCall(ASTNode* node, bool keepFunctionResult = false);

    void genExpression(ASTNode* node);
    void genSimpleExpression(ASTNode* node);
    void genTerm(ASTNode* node);
    void genFactor(ASTNode* node);
    void genConstant(ASTNode* node);
    void genIndexValue(ASTNode* indexNode);
    std::vector<ASTNode*> flattenIndexList(ASTNode* indexList) const;

    // Variable addressing
    void genAddress(ASTNode* lhsOrVariable);
    void genVariableValue(ASTNode* node);
    VarLocation locationForIdent(ASTNode* ident) const;
    int fieldOffset(int recordRef, const std::string& fieldName, int* outType = nullptr, int* outRef = nullptr) const;
};

#endif
