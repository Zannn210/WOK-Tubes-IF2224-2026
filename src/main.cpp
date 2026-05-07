#include <iostream>
#include <string>
#include <exception>
#include <filesystem>
#include "ArionLexer.hpp"
#include "Parser.hpp"

namespace {
std::string outputPath(const std::string& directory, const std::string& prefix, int number) {
    return directory + "/" + prefix + std::to_string(number) + ".txt";
}

int nextOutputNumber(const std::string& directory) {
    std::filesystem::create_directories(directory);

    int number = 1;
    while (true) {
        if (!std::filesystem::exists(outputPath(directory, "token_output_", number)) &&
            !std::filesystem::exists(outputPath(directory, "tree_output_", number))) {
            return number;
        }
        number++;
    }
}
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Penggunaan: " << argv[0] << " <input_source.txt> [output_token.txt] [output_tree.txt]\n";
        return 1;
    }

    std::string inputPath  = argv[1];
    std::string outputDir = "test/milestone-2";
    int outputNumber = nextOutputNumber(outputDir);
    std::string tokenPath  = (argc >= 3) ? argv[2] : outputPath(outputDir, "token_output_", outputNumber);
    std::string treePath   = (argc >= 4) ? argv[3] : outputPath(outputDir, "tree_output_", outputNumber);

    std::cout << "Menyimpan token ke " << tokenPath << "\n";
    std::cout << "Menyimpan parse tree ke " << treePath << "\n";

    {
        ArionLexer lexer(inputPath, tokenPath);
        if (!lexer.isOpen()) {
            std::cerr << "[ERROR] Tidak dapat membuka file: "<< inputPath << "\n";
            return 1;
        }
        lexer.analyze();
    }
    std::cout << "\n=== Lexical Analysis Selesai ===\n\n";

    try {
        std::vector<Token> tokenList = loadTokensFromFile(tokenPath);
        Parser parser(tokenList, treePath);

        std::cout << "=== Parse Tree ===\n";
        parser.parse();
        std::cout << "\n[SUCCESS] Parsing selesai. Hasil disimpan di " << treePath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
