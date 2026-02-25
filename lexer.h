#ifndef LEXER_H
#define LEXER_H

/*
State-Machine Lexer

Lexing/tokenizing turns an input string into a
stream of tokens, which hold some kind of meaning.

It's a weird in-between step, but it's necessary
in order to implement the parser because without
tokens, we'd just be iterating through chars in a
string. Imagine trying to parse "3.14 *2" one letter
at a time. Yeah fuck no

The state-machine here is used because it's the
best way to model lexical rules. If digit, go to
number state, if '\', go to command state, etc.

It will traverse through the string, deciding what
token type to make based on previous/next characters,
and then build that token one by one until something
that can't be in that token type is seen.

    The state of this lexer when parsing "3.14 *2":
        - sees 3, number state
        - sees ., fraction mark state
        - sees 1, fraction state
        - sees 4, fraction state
        - sees space, end and commit that fraction token, start state
        - sees *, commit a star token, start state
        - sees 2, number state
        - sees '\0', commit the current token, add an end token, and return

The parser can now work with a stream of useful info,
instead of having to sift through an ambiguous string
of characters to extract meaning.
*/

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
    Equals,
    End
};

struct Token {
    TokenType type;
    std::string lexeme;
    size_t pos;
    std::optional<Number> number;

    bool is(TokenType t) const { return type == t; }

    // overloads from number for readability in parser
    decltype(auto) value() { return number->value; }
    bool isInt() const { return number.has_value() && number->isInt; }
};

bool Tokenize(const std::string& input, std::vector<Token>& tokens);

#endif