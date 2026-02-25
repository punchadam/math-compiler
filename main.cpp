#include "lexer.h"
#include <iostream>

void getInputString(std::string& input) {
    std::cout << "Math Compiler v0.0.0\n\n\tInput Expression:\t";
    std::cin >> input;
    return;
}

int main(void) {
    std::string input;
    getInputString(input);

    std::vector<Token> tokens;
    if (!Tokenize(input, tokens)) return 1;

    std::cout << "\nTokens:\n";
    for (Token t : tokens) {
        std::cout << t.lexeme << "\n";
    }
    return 0;
}