#include <iostream>
#include <string>
#include "ArionLexer.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Penggunaan: " << argv[0] << " <input.txt> [output.txt]\n";
        return 1;
    }

    std::string inputPath  = argv[1];
    std::string outputPath = (argc >= 3) ? argv[2] : "";

    ArionLexer lexer(inputPath, outputPath);

    if (!lexer.isOpen()) {
        std::cerr << "[ERROR] Tidak dapat membuka file: "<< inputPath << "\n";
        return 1;
    }

    lexer.analyze();
    return 0;
}
