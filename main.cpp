#include "lexer.h"
//#include <sstream>
#include <iostream>

int main(void) {
    std::string input = "\\frac{1}{2}+\\sin{30}";

    std::vector<Token> tokens;
    if (!Tokenize(input, tokens)) return 0;

    std::cout << "\nOriginal String: " << input;
    std::cout << "\nTokens:\n";
    for (Token t : tokens) {
    }

    return 0;
}