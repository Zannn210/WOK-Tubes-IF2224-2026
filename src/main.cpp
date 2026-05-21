#include <iostream>
#include <string>
#include <exception>
#include <filesystem>
#include "ArionLexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"

namespace {

std::string outputPath(const std::string& dir, const std::string& prefix, int n) {
    return dir + "/" + prefix + std::to_string(n) + ".txt";
}

int nextOutputNumber(const std::string& dir) {
    std::filesystem::create_directories(dir);
    int n = 1;
    while (std::filesystem::exists(outputPath(dir, "token_output_", n)) ||
           std::filesystem::exists(outputPath(dir, "tree_output_",  n)) ||
           std::filesystem::exists(outputPath(dir, "ast_output_",   n))) {
        n++;
    }
    return n;
}

} 

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <source.txt> [token_out.txt] [tree_out.txt] [ast_out.txt]\n";
        return 1;
    }

    const std::string inputPath = argv[1];
    const std::string outputDir = "test/milestone-3";
    const int n = nextOutputNumber(outputDir);
    const std::string tokenPath = (argc >= 3) ? argv[2] : outputPath(outputDir, "token_output_", n);
    const std::string treePath  = (argc >= 4) ? argv[3] : outputPath(outputDir, "tree_output_",  n);
    const std::string astPath   = (argc >= 5) ? argv[4] : outputPath(outputDir, "ast_output_",   n);

    std::cout << "Token output : " << tokenPath << "\n" << "Parse tree   : " << treePath  << "\n"  << "Semantic AST : " << astPath   << "\n\n";

    {
        ArionLexer lexer(inputPath, tokenPath);
        if (!lexer.isOpen()) {
            std::cerr << "[ERROR] Cannot open source file: " << inputPath << "\n";
            return 1;
        }
        lexer.analyze();
    }
    std::cout << "=== Lexical Analysis Done ===\n\n";

    ASTNode* parseTree = nullptr;
    try {
        std::vector<Token> tokenList = loadTokensFromFile(tokenPath);
        Parser parser(tokenList, treePath);

        std::cout << "=== Parse Tree ===\n";
        parseTree = parser.parse();
        std::cout << "\n[SUCCESS] Parsing done. Tree saved to " << treePath << "\n\n";
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }

    try {
        SemanticAnalyzer sa;
        std::cout << "=== Semantic Analysis ===\n";
        sa.analyze(parseTree, astPath);
        std::cout << "\n[SUCCESS] Semantic analysis done. AST saved to " << astPath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[SEMANTIC ERROR] " << e.what() << "\n";
        delete parseTree;
        return 1;
    }

    delete parseTree;
    return 0;
}
