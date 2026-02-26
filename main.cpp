#include "lexer.h"
#include "parser.h"
#include <iostream>

bool getInputString(std::string& input) {
    std::cout << "Math Compiler v0.0.0\n\n\tInput Expression:\t";
    std::cin >> input;
    if (input == "n" || input == "no" || input == "N" || input == "No" || input == "NO") return false;
    return true;
}

int main(void) {
    std::string input;
    Parser parser;
    AST ast;

    while (getInputString(input)) {
        std::vector<Token> tokens;
        if (!Tokenize(input, tokens)) return 1;
        std::cout << "\nTokens:\n";
        for (Token t : tokens) {
            std::cout << t.lexeme << "\n";
        }
        parser.parse(tokens, ast);
    }
    return 0;
}