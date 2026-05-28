#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <filesystem>
#include "ArionLexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "IntermediateCodeGenerator.hpp"
#include "StackInterpreter.hpp"
#include <fstream>

namespace {

std::string nthOutput(const std::string& dir, const std::string& prefix, int n) {
    return dir + "/" + prefix + std::to_string(n) + ".txt";
}

int nextN(const std::string& dir) {
    std::filesystem::create_directories(dir);
    int n = 1;
    while (std::filesystem::exists(nthOutput(dir, "token_output_", n))) n++;
    return n;
}

std::vector<std::string> listInputFiles(const std::string& dir) {
    std::vector<std::string> files;
    if (!std::filesystem::exists(dir)) return files;
    for (auto& e : std::filesystem::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        if (e.path().extension() != ".txt") continue;
        std::string name = e.path().filename().string();
        if (name.rfind("token_",  0) == 0) continue;
        if (name.rfind("tree_",   0) == 0) continue;
        if (name.rfind("ast_",    0) == 0) continue;
        if (name.find("output")  != std::string::npos) continue;
        files.push_back(name);
    }
    std::sort(files.begin(), files.end());
    return files;
}

int runAll(const std::string& inputPath, const std::string& milestoneDir, int milestoneNum) {
    const int n = nextN(milestoneDir);
    const std::string tokenPath = nthOutput(milestoneDir, "token_output_", n);
    const std::string treePath  = nthOutput(milestoneDir, "tree_output_",  n);
    const std::string astPath   = nthOutput(milestoneDir, "ast_output_",   n);
    const std::string icPath    = nthOutput(milestoneDir, "ic_output_",    n);
    const std::string runPath   = nthOutput(milestoneDir, "run_output_",   n);

    std::cout << "\nFile input   : " << inputPath << "\n";
    std::cout << "Token output : " << tokenPath << "\n";
    if (milestoneNum >= 2) std::cout << "Parse tree   : " << treePath  << "\n";
    if (milestoneNum >= 3) std::cout << "Semantic AST : " << astPath   << "\n";
    if (milestoneNum >= 4) {
        std::cout << "IC output    : " << icPath    << "\n";
        std::cout << "Run output   : " << runPath   << "\n";
    }
    std::cout << "\n";

    // Phase 1: Lexer
    std::vector<int> tokenLines;
    {
        ArionLexer lexer(inputPath, tokenPath);
        if (!lexer.isOpen()) {
            std::cerr << "[ERROR] Cannot open source file: " << inputPath << "\n";
            return 1;
        }
        lexer.analyze();
        tokenLines = lexer.getTokenLines();
    }
    std::cout << "=== Lexical Analysis Done ===\n\n";
    if (milestoneNum < 2) return 0;

    // Phase 2: Parser
    ASTNode* parseTree = nullptr;
    try {
        std::vector<Token> tokenList = loadTokensFromFile(tokenPath, tokenLines);
        Parser parser(tokenList, treePath);
        std::cout << "=== Parse Tree ===\n";
        parseTree = parser.parse();
        std::cout << "\n[SUCCESS] Parsing done. Tree saved to " << treePath << "\n\n";
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }
    if (milestoneNum < 3) { delete parseTree; return 0; }

    // Phase 3: Semantic Analysis
    try {
        SemanticAnalyzer sa;
        std::cout << "=== Semantic Analysis ===\n";
        sa.analyze(parseTree, astPath);
        std::cout << "\n[SUCCESS] Semantic analysis done. AST saved to " << astPath << "\n";

        // Phase 4: Intermediate Code Generation + Stack-Machine Interpreter
        if (milestoneNum >= 4) {
            std::cout << "\n=== Intermediate Code Generation ===\n";
            IntermediateCodeGenerator icg(sa.getTab(), sa.getBtab(), sa.getAtab());
            Code code = icg.generate(parseTree, icPath);
            std::cout << "[SUCCESS] IC saved to " << icPath << " (" << code.size() << " instructions)\n";

            std::cout << "\n=== Program Output ===\n";
            StackInterpreter vm(code);
            vm.run(std::cout);

            std::ofstream runFile(runPath);
            if (runFile.is_open()) {
                StackInterpreter vmFile(code);
                vmFile.run(runFile);
                std::cout << "\n[SUCCESS] Runtime output saved to " << runPath << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        delete parseTree;
        return 1;
    }

    delete parseTree;
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    // Direct invocation: ./arion_lexer <source.txt>
    if (argc >= 2) {
        const std::string inputPath = argv[1];
        const std::string outputDir = "test/milestone-4";
        return runAll(inputPath, outputDir, 4);
    }

    // Interactive mode
    const std::string testRoot = "test";

    // Step 1: list milestone directories
    std::vector<std::string> milestones;
    if (std::filesystem::exists(testRoot)) {
        for (auto& e : std::filesystem::directory_iterator(testRoot))
            if (e.is_directory())
                milestones.push_back(e.path().filename().string());
        std::sort(milestones.begin(), milestones.end());
    }
    if (milestones.empty()) {
        std::cerr << "Folder 'test/' tidak ditemukan atau kosong.\n";
        return 1;
    }

    std::cout << "=== Arion Compiler ===\n\n";
    std::cout << "Pilih milestone:\n";
    for (int i = 0; i < (int)milestones.size(); i++)
        std::cout << "  " << (i + 1) << ". " << milestones[i] << "\n";
    int msChoice = 0;
    while (true) {
        std::cout << "Pilihan (1-" << milestones.size() << "): ";
        if (std::cin >> msChoice && msChoice >= 1 && msChoice <= (int)milestones.size()) break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [!] Pilihan tidak valid. Masukkan angka 1-" << milestones.size() << ".\n";
    }

    const std::string milestoneName = milestones[msChoice - 1];
    const std::string milestoneDir  = testRoot + "/" + milestoneName;

    int milestoneNum = 0;
    if (milestoneName.size() > 10 && milestoneName.rfind("milestone-", 0) == 0) {
        try { milestoneNum = std::stoi(milestoneName.substr(10)); }
        catch (...) {}
    }

    // Step 2: list input files
    auto files = listInputFiles(milestoneDir);
    if (files.empty()) {
        std::cerr << "Tidak ada file input di " << milestoneDir << "\n";
        return 1;
    }

    std::cout << "\nPilih file input:\n";
    for (int i = 0; i < (int)files.size(); i++)
        std::cout << "  " << (i + 1) << ". " << files[i] << "\n";
    int fileChoice = 0;
    while (true) {
        std::cout << "Pilihan (1-" << files.size() << "): ";
        if (std::cin >> fileChoice && fileChoice >= 1 && fileChoice <= (int)files.size()) break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [!] Pilihan tidak valid. Masukkan angka 1-" << files.size() << ".\n";
    }

    const std::string inputPath = milestoneDir + "/" + files[fileChoice - 1];
    return runAll(inputPath, milestoneDir, milestoneNum);
}
