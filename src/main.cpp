#include <iostream>
#include <string>
#include "ArionLexer.hpp"
#include "Parser.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Penggunaan: " << argv[0] << " <input_source.txt> <output_token.txt> [output_tree.txt]\n";
        return 1;
    }

    std::string inputPath  = argv[1];
    std::string tokenPath  = argv[2];
    std::string treePath   = (argc >= 4) ? argv[3] : "parsetree_output.txt";

    // 1. Jalankan Lexer
    ArionLexer lexer(inputPath, tokenPath);
    if (!lexer.isOpen()) {
        std::cerr << "[ERROR] Tidak dapat membuka file: "<< inputPath << "\n";
        return 1;
    }
    lexer.analyze();
    std::cout << "\n=== Lexical Analysis Selesai ===\n\n";

    // 2. Jalankan Parser
    std::vector<Token> tokenList = loadTokensFromFile(tokenPath);
    Parser parser(tokenList, treePath);
    
    std::cout << "=== Parse Tree ===\n";
    parser.parse();
    
    std::cout << "\n[SUCCESS] Parsing selesai. Hasil disimpan di " << treePath << "\n";

    return 0;
}