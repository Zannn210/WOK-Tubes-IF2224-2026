#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ArionLexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "IntermediateCodeGenerator.hpp"
#include "StackInterpreter.hpp"

namespace fs = std::filesystem;

namespace {

std::string sanitizeName(std::string name) {
    for (char& ch : name) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_' && ch != '-') {
            ch = '_';
        }
    }
    return name.empty() ? "program" : name;
}

std::string caseNameFromInput(const std::string& inputPath) {
    return sanitizeName(fs::path(inputPath).stem().string());
}

fs::path caseOutputDir(const std::string& milestoneDir, const std::string& caseName) {
    return fs::path(milestoneDir) / "output" / caseName;
}

std::string outputFile(const fs::path& outDir, const std::string& prefix, const std::string& caseName) {
    return (outDir / (prefix + "_" + caseName + ".txt")).string();
}

void writeTextFile(const std::string& path, const std::string& text) {
    std::ofstream file(path);
    if (file.is_open()) file << text;
}

std::vector<std::string> listInputFiles(const std::string& dir) {
    std::vector<std::string> files;
    if (!fs::exists(dir)) return files;

    for (auto& e : fs::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        if (e.path().extension() != ".txt") continue;

        std::string name = e.path().filename().string();
        if (name.rfind("token_", 0) == 0) continue;
        if (name.rfind("parse_tree_", 0) == 0) continue;
        if (name.rfind("tree_", 0) == 0) continue;
        if (name.rfind("decorated_ast_", 0) == 0) continue;
        if (name.rfind("ast_", 0) == 0) continue;
        if (name.rfind("intermediate_code_", 0) == 0) continue;
        if (name.rfind("ic_", 0) == 0) continue;
        if (name.rfind("run_", 0) == 0) continue;
        if (name.rfind("expected_", 0) == 0) continue;
        if (name.find("output") != std::string::npos) continue;

        files.push_back(name);
    }

    std::sort(files.begin(), files.end());
    return files;
}

int runAll(const std::string& inputPath, const std::string& milestoneDir, int milestoneNum) {
    const std::string caseName = caseNameFromInput(inputPath);
    const fs::path outDir = caseOutputDir(milestoneDir, caseName);
    fs::create_directories(outDir);

    const std::string tokenPath = outputFile(outDir, "token", caseName);
    const std::string treePath  = outputFile(outDir, "parse_tree", caseName);
    const std::string astPath   = outputFile(outDir, "decorated_ast", caseName);
    const std::string icPath    = outputFile(outDir, "intermediate_code", caseName);
    const std::string runPath   = outputFile(outDir, "run", caseName);

    std::cout << "\nFile input   : " << inputPath << "\n";
    std::cout << "Output folder: " << outDir.string() << "\n";
    std::cout << "Token output : " << tokenPath << "\n";
    if (milestoneNum >= 2) std::cout << "Parse tree   : " << treePath << "\n";
    if (milestoneNum >= 3) std::cout << "Semantic AST : " << astPath << "\n";
    if (milestoneNum >= 4) {
        std::cout << "IC output    : " << icPath << "\n";
        std::cout << "Run output   : " << runPath << "\n";
    }
    std::cout << "\n";

    // Fase 1: lexer baca source mentah lalu keluarin token
    std::vector<int> tokenLines;
    {
        ArionLexer lexer(inputPath, tokenPath);
        if (!lexer.isOpen()) {
            const std::string msg = "[ERROR] Cannot open source file: " + inputPath + "\n";
            std::cerr << msg;
            if (milestoneNum >= 4) writeTextFile(runPath, msg);
            return 1;
        }
        lexer.analyze();
        tokenLines = lexer.getTokenLines();
    }
    std::cout << "=== Lexical Analysis Done ===\n\n";
    if (milestoneNum < 2) return 0;

    // Fase 2: parser ubah token jadi parse tree
    ASTNode* parseTree = nullptr;
    try {
        std::vector<Token> tokenList = loadTokensFromFile(tokenPath, tokenLines);
        Parser parser(tokenList, treePath);
        std::cout << "=== Parse Tree ===\n";
        parseTree = parser.parse();
        std::cout << "\n[SUCCESS] Parsing done. Tree saved to " << treePath << "\n\n";
    } catch (const std::exception& e) {
        const std::string msg = std::string("[PARSE ERROR] ") + e.what() + "\n";
        std::cerr << msg;
        if (milestoneNum >= 4) writeTextFile(runPath, msg);
        return 1;
    }
    if (milestoneNum < 3) { delete parseTree; return 0; }

    // Fase 3: semantic analyzer kasih dekorasi tipe, scope, dan symbol table
    try {
        SemanticAnalyzer sa;
        std::cout << "=== Semantic Analysis ===\n";
        sa.analyze(parseTree, astPath);
        std::cout << "\n[SUCCESS] Semantic analysis done. AST saved to " << astPath << "\n";

        // Fase 4: dari Decorated AST ke instruksi stack machine, lalu dieksekusi VM
        if (milestoneNum >= 4) {
            std::cout << "\n=== Intermediate Code Generation ===\n";
            IntermediateCodeGenerator icg(sa.getTab(), sa.getBtab(), sa.getAtab());
            Code code = icg.generate(parseTree, icPath);
            std::cout << "[SUCCESS] IC saved to " << icPath << " (" << code.size() << " instructions)\n";

            std::cout << "\n=== Program Output ===\n";

            std::ostringstream runtimeOutput;
            try {
                StackInterpreter vm(code);
                vm.run(runtimeOutput);

                std::cout << runtimeOutput.str();
                writeTextFile(runPath, runtimeOutput.str());
                std::cout << "\n[SUCCESS] Runtime output saved to " << runPath << "\n";
            } catch (const std::exception& e) {
                runtimeOutput << "\n[RUNTIME ERROR] " << e.what() << "\n";
                std::cout << runtimeOutput.str();
                writeTextFile(runPath, runtimeOutput.str());
                delete parseTree;
                return 1;
            }
        }
    } catch (const std::exception& e) {
        const std::string msg = std::string("[ERROR] ") + e.what() + "\n";
        std::cerr << msg;
        if (milestoneNum >= 4 && !fs::exists(runPath)) writeTextFile(runPath, msg);
        delete parseTree;
        return 1;
    }

    delete parseTree;
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        const std::string inputPath = argv[1];
        fs::path parent = fs::path(inputPath).parent_path();
        const std::string outputBaseDir = parent.empty() ? "." : parent.string();
        return runAll(inputPath, outputBaseDir, 4);
    }

    const std::string testRoot = "test";

    std::vector<std::string> milestones;
    if (fs::exists(testRoot)) {
        for (auto& e : fs::directory_iterator(testRoot)) {
            if (e.is_directory()) milestones.push_back(e.path().filename().string());
        }
        std::sort(milestones.begin(), milestones.end());
    }

    if (milestones.empty()) {
        std::cerr << "Folder 'test/' tidak ditemukan atau kosong.\n";
        return 1;
    }

    std::cout << "=== Arion Compiler ===\n\n";
    std::cout << "Pilih milestone:\n";
    for (int i = 0; i < static_cast<int>(milestones.size()); i++) {
        std::cout << "  " << (i + 1) << ". " << milestones[i] << "\n";
    }

    int msChoice = 0;
    while (true) {
        std::cout << "Pilihan (1-" << milestones.size() << "): ";
        if (std::cin >> msChoice && msChoice >= 1 && msChoice <= static_cast<int>(milestones.size())) break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [!] Pilihan tidak valid. Masukkan angka 1-" << milestones.size() << ".\n";
    }

    const std::string milestoneName = milestones[msChoice - 1];
    const std::string milestoneDir  = testRoot + "/" + milestoneName;

    int milestoneNum = 0;
    if (milestoneName.size() > 10 && milestoneName.rfind("milestone-", 0) == 0) {
        try { milestoneNum = std::stoi(milestoneName.substr(10)); }
        catch (...) { milestoneNum = 4; }
    }

    auto files = listInputFiles(milestoneDir);
    if (files.empty()) {
        std::cerr << "Tidak ada file input di " << milestoneDir << "\n";
        return 1;
    }

    std::cout << "\nPilih file input:\n";
    for (int i = 0; i < static_cast<int>(files.size()); i++) {
        std::cout << "  " << (i + 1) << ". " << files[i] << "\n";
    }

    int fileChoice = 0;
    while (true) {
        std::cout << "Pilihan (1-" << files.size() << "): ";
        if (std::cin >> fileChoice && fileChoice >= 1 && fileChoice <= static_cast<int>(files.size())) break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [!] Pilihan tidak valid. Masukkan angka 1-" << files.size() << ".\n";
    }

    const std::string inputPath = milestoneDir + "/" + files[fileChoice - 1];
    return runAll(inputPath, milestoneDir, milestoneNum);
}
