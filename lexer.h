#ifndef LEXER_H
#define LEXER_H

#include "lookupstuff.h"

enum class TokenType {
    Number,
    Identifier,
    Command,
    LBrace, RBrace,
    LParenthesis, RParenthesis,
    LBracket, RBracket,
    Comma,
    Plus, Minus, Star, Slash,
    Caret, Underscore,
    Equals
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::optional<size_t> pos;
    std::optional<Number> number;

    bool is(TokenType t) const { return type == t; }

    // overloads from number for readability in parser
    decltype(auto) value() { return number->value; }
    bool isInt() { return number.has_value() && number->isInt; }
};

bool Tokenize(const std::string& input, std::vector<Token>& tokens);

#endif