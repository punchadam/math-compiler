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
    Parser parser;
    AST ast;

    i64 n, d;
    double test = 0.125;
    std::cout << "Fraction Representation of Number 0.125:\n";
    if (parser.doubleToRational(test, n, d)) std::cout << n << "/" << d << '\n';
    else std::cout << "Unable to parse float.\n";
    
    std::string input;
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