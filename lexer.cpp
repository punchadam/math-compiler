#include "lexer.h"

enum class State {
    Start,
    Number,
    NumberFracMark,
    NumberFrac,
    NumberExpMark,
    NumberExpSign,
    NumberExp,
    Identifier,
    Command
};

bool Tokenize(const std::string& input, std::vector<Token>& tokens) {
    State s = State::Start;

    std::string buffer; // stored lexeme
    size_t startPos = 0;   // start of token in original string

    auto commit = [&](TokenType type, std::string lexeme, size_t pos, std::optional<Number> number = std::nullopt) {
        if (number.has_value()) { tokens.push_back(Token{ type, std::move(lexeme), pos, number }); }
        else { tokens.push_back(Token{ type, std::move(lexeme), pos }); }
    };

    // begin a new token
    auto begin = [&](State next, size_t pos) {
        s = next;
        buffer.clear();
        startPos = pos;
    };

    for (size_t i = 0; i <= input.size(); i++) {
        // add a sentinel '\0' at the end of the input string
        unsigned char c = (i < input.size()) ? static_cast<unsigned char>(input[i]) : '\0';

        switch (s) {
            case State::Start: {
                if (c == '\0') {
                    // End sentinel token at the end + 1
                    commit(TokenType::End, "", input.size());
                    return true;
                }

                // ignore spaces at the start of a token
                if (isspace(c)) { break; }
                
                if (isdigit(c)) { begin(State::Number, i); buffer += static_cast<char>(c); break; }
                if (c == '.') { begin(State::NumberFracMark, i); buffer += '.'; break; }
                if (c == '\\') { begin(State::Command, i); buffer += '\\'; break; }
                
                if (isalnum(c)) { begin(State::Identifier, i); buffer += static_cast<char>(c); break; }
                
                if (c == '{') { commit(TokenType::LBrace, "{", i); break; }
                if (c == '}') { commit(TokenType::RBrace, "}", i); break; }
                if (c == '(') { commit(TokenType::LParenthesis, "(", i); break; }
                if (c == ')') { commit(TokenType::RParenthesis, ")", i); break; }
                if (c == '[') { commit(TokenType::LBracket, "[", i); break; }
                if (c == ']') { commit(TokenType::RBracket, "]", i); break; }
                
                if (c == ',') { commit(TokenType::Comma, ",", i); break; }
                if (c == '+') { commit(TokenType::Plus, "+", i); break; }
                if (c == '-') { commit(TokenType::Minus, "-", i); break; }
                if (c == '*') { commit(TokenType::Star, "*", i); break; }
                if (c == '/') { commit(TokenType::Slash, "/", i); break; }
                if (c == '^') { commit(TokenType::Caret, "^", i); break; }
                if (c == '_') { commit(TokenType::Underscore, "_", i); break; }
                if (c == '=') { commit(TokenType::Equals, "=", i); break; }

                return false;
            }

            case State::Number: {
                if (isdigit(c)) { buffer += c; break; }
                if (c == '.') { buffer += '.'; s = State::NumberFracMark; break; }

                if (c == 'e' || c == 'E') { buffer += static_cast<char>(c); s = State::NumberExpMark; break; }
                
                Number num{ std::stoi(buffer), true };  // state stayed within number, commit an int
                commit(TokenType::Number, buffer, i, num);
                s = State::Start;
                if (c != '\0') --i;
                break;
            }
            case State::NumberFracMark: {
                if (isdigit(c)) { buffer += static_cast<char>(c); s = State::NumberFrac; break; }
                return false;
            }
            case State::NumberFrac: {
                if (isdigit(c)) { buffer += static_cast<char>(c); break; }
                
                Number num{ static_cast<i64>(std::stoll(buffer)), true };
                commit(TokenType::Number, buffer, i, num);
                s = State::Start;
                if (c != '\0') --i;
                break;
            }
            case State::NumberExpMark: {
                if (isdigit(c)) { buffer += static_cast<char>(c); s = State::NumberExp; break; }
                if (c == '-') { buffer += '-'; s = State::NumberExpSign; break; }
                return false;
            }
            case State::NumberExpSign: {
                if (isdigit(c)) { buffer += static_cast<char>(c); s = State::NumberExp; break; }
                return false;
            }
            case State::NumberExp: {
                if (isdigit(c)) { buffer += static_cast<char>(c); break; }
                
                Number num{ std::stod(buffer), false };
                commit(TokenType::Number, buffer, i, num);
                s = State::Start;
                if (c != '\0') --i;
                break;
            }
            case State::Identifier: {
                if (isalnum(c)) { buffer += c; break; } 

                commit(TokenType::Identifier, buffer, startPos);
                s = State::Start;
                if (c != '\0') --i;
                break;
            }
            case State::Command: {
                if (isalnum(c)) { buffer += c; break; }

                commit(TokenType::Command, buffer, startPos);
                s = State::Start;
                if (c != '\0') --i;
                break;
            }
        }
    }
    // after the loop, back to the start means it all got handled
    if (s == State::Start) {
        return true;
    }

    return false;
}