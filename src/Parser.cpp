#include "Parser.hpp"
#include <iostream>
#include <cstdlib>

//utility dan file loader

std::vector<Token> loadTokensFromFile(const std::string& filename) {
    std::vector<Token> tokens;
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        Token t;
        size_t pos = line.find(" (");
        if (pos != std::string::npos) {
            t.type = line.substr(0, pos);
            t.value = line.substr(pos + 2, line.length() - pos - 3);
        } else {
            t.type = line;
            t.value = "";
        }
        tokens.push_back(t);
    }
    return tokens;
}

Parser::Parser(const std::vector<Token>& t, const std::string& outFilename) 
    : tokens(t), currentIndex(0), indentLevel(0) {
    outFile.open(outFilename);
}

Parser::~Parser() {
    if (outFile.is_open()) outFile.close();
}

Token Parser::currentToken() {
    if (currentIndex < (int)tokens.size()) return tokens[currentIndex];
    return {"EOF", ""}; 
}

Token Parser::lookahead(int offset) {
    if (currentIndex + offset < (int)tokens.size()) return tokens[currentIndex + offset];
    return {"EOF", ""};
}

void Parser::advance() {
    if (currentIndex < (int)tokens.size()) currentIndex++;
}

void Parser::printNode(const std::string& nodeName) {
    std::string indent = "";
    for(int i = 0; i < indentLevel; i++) indent += "|   ";
    if (indentLevel > 0) indent += "|-- ";

    std::cout << indent << nodeName << "\n";
    if (outFile.is_open()) {
        outFile << indent << nodeName << "\n";
    }
}

bool Parser::match(const std::string& expectedType) {
    Token t = currentToken();
    if (t.type == expectedType) {
        std::string printText = t.type;
        if (!t.value.empty()) printText += " (" + t.value + ")";
        
        printNode(printText);
        advance();
        return true;
    } else {
        std::cerr << "\n[SYNTAX ERROR] Unexpected token '" << t.type 
                  << "', expected '" << expectedType << "'\n";
        exit(1); 
    }
}

void Parser::parse() {
    parseProgram();
}

//root program

void Parser::parseProgram() {
    printNode("<program>");
    indentLevel++;
    
    parseProgramHeader();
    // parseDeclarationPart(); 
    // parseCompoundStatement(); 
    match("period");
    
    indentLevel--;
}

void Parser::parseProgramHeader() {
    printNode("<program-header>");
    indentLevel++;
    
    match("programsy");
    match("ident");
    match("semicolon");
    
    indentLevel--;
}