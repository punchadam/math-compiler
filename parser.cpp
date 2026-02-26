#include "parser.h"

#include <stdexcept>

bool Parser::parse(const std::vector<Token>& tokens, AST& outAST) {
    _tokens = &tokens;
    _ast = &outAST;

    _ast->root = parseExpression(0);

    return peek().is(TokenType::End);
}


const Token& Parser::peek() const {
    return (*_tokens)[_pos];
}

const Token& Parser::advance() {
    const Token& t = (*_tokens)[_pos];
    if (!t.is(TokenType::End)) _pos++;
    return t;
}

const Token& Parser::expect(const TokenType& type) {
    if ((*_tokens)[_pos].type == type) {
        return advance();
    }
    throw std::runtime_error("expected " + std::to_string((int)type) + ", got '" + peek().lexeme + "'");
}

NodeID Parser::parseExpression(const u8& minBP) {
    NodeID leftSide = parsePrefix();

    while (true) {
        const Token& t = peek();

        // postfix ops
        if (t.is(TokenType::Identifier) && t.lexeme == "!") {
            if (POSTFIX_LBP < minBP) break;
            size_t p = advance().pos;
            leftSide = _ast->addUnaryOp(UnaryOpKind::Factorial, leftSide, p);
        }

        // infix ops
        if (auto it = INFIX_OPS.find(t.type); it != INFIX_OPS.end()) {
            auto [leftBP, rightBP, opKind] = it->second;
            if (leftBP < minBP) break;
            const Token& op = advance();
            NodeID rightSide = parseExpression(rightBP);
            leftSide = _ast->addBinaryOp(opKind, leftSide, rightSide, op.pos);
            continue;
        }

        // infix commands
        if (t.is(TokenType::Command) &&
            (t.lexeme == "cdot" || t.lexeme == "times" || t.lexeme == "div"))
        {
            if (5 < minBP) break;
            BinaryOpKind kind = (t.lexeme == "div")
                ? BinaryOpKind::Divide : BinaryOpKind::Multiply;
            size_t p = advance().pos;
            NodeID rightSide = parseExpression(6);
            leftSide = _ast->addBinaryOp(kind, leftSide, rightSide, p);
            continue;
        }

        // implicit multiplication
        if (canImplicitMultiply()) {
            auto [leftBP, rightBP, opKind] = IMPLICIT_MULTIPLY_OP;
            if (leftBP < minBP) break;
            NodeID rightSide = parseExpression(rightBP);
            leftSide = _ast->addBinaryOp(opKind, leftSide, rightSide, peek().pos);
            continue;
        }

        break;
    }

    return leftSide;
}

// numbers, identifiers, unary '-', groups, commands
NodeID Parser::parsePrefix() {
    const Token& t = peek();

    if (t.is(TokenType::Number)) return parseNumber();

    if (t.is(TokenType::Identifier)) {
        const Token& t = advance();
        return _ast->addIdentifier(t.lexeme, t.pos);
    }

    if (t.is(TokenType::Minus)) {
        size_t p = advance().pos;
        NodeID inner = parseExpression(PREFIX_UNARY_RBP);
        return _ast->addUnaryOp(UnaryOpKind::Negate, inner, p);
    }

    if (t.is(TokenType::LParenthesis)) {
        advance();
        NodeID inner = parseExpression(0);
        expect(TokenType::RParenthesis);
        return inner;
    }

    if (t.is(TokenType::RBrace)) {
        return parseBraceGroup();
    }

    if (t.is(TokenType::Command)) {
        return parseCommand();
    }

    throw std::runtime_error("unexpected token at pos " + std::to_string(t.pos));
}

NodeID Parser::parseNumber() {
    const Token& t = peek();
    if (t.isInt()) {
        return _ast->addRational(std::get<i64>(t.number->value), 1, t.pos);
    }
    return _ast->addReal(std::get<double>(t.number->value), t.pos);
}

NodeID Parser::parseCommand() {
    const Token& t = peek();
    const std::string& cmd = t.lexeme;

    if (auto it = CONSTANT_MAP.find(cmd); it != CONSTANT_MAP.end()) {
        size_t p = advance().pos;
        return _ast->addConstant(it->second, p);
    }

    if (auto it = FUNCTION_KIND_MAP.find(cmd); it != FUNCTION_KIND_MAP.end()) {
        return parseSingleArgFunction();
    }

    if (cmd == "operatorname") {
        return parseOperatorName();
    }

    if (cmd == "frac") {
        return parseFraction();
    }

    if (cmd == "sqrt") {
        size_t p = advance().pos;
        NodeID inner = parseBraceGroup();

        // sqrt(x) = x^(1/2)
        NodeID half = _ast->addRational(1, 2, p);
        return _ast->addBinaryOp(BinaryOpKind::Power, inner, half, p);
    }

    if (cmd == "left") {
        return parseLeftRight();
    }

    throw std::runtime_error("unknown command: \\" + cmd);
}

NodeID Parser::parseBraceGroup() {
    expect(TokenType::LBrace);
    NodeID inner = parseExpression(0);
    expect(TokenType::RBrace);
    return inner;
}

NodeID Parser::parseLeftRight() {
    advance();
    expect(TokenType::LParenthesis);
    NodeID inner = parseExpression(0);
    if (!peek().is(TokenType::Command) || peek().lexeme != "right") {
        throw std::runtime_error("expected \\right");
    }
    advance();
    expect(TokenType::RParenthesis);
    return inner;
}

NodeID Parser::parseFraction() {
    size_t p = advance().pos;

    // store the original pos so we can use advance freely
    size_t saved = _pos;

    // fast path to a rational node if both n & d are integers
    if (peek().is(TokenType::LBrace)) {
        advance();
        bool negativeNumerator = peek().is(TokenType::Minus);
        if (negativeNumerator) { advance(); }
        if (peek().is(TokenType::Number) && peek().isInt()) {
            i64 numerator = std::get<i64>(advance().number->value);
            if (negativeNumerator) numerator = -numerator;
            if (peek().is(TokenType::RBrace)) {
                advance();
                if (peek().is(TokenType::LBrace)) {
                    advance();
                    bool negativeDenominator = peek().is(TokenType::Minus);
                    if (negativeDenominator) { advance(); }
                    if (peek().is(TokenType::Number) && peek().isInt())  {
                        i64 denominator = std::get<i64>(advance().number->value);
                        if (negativeDenominator) denominator = -denominator;
                        if (peek().is(TokenType::RBrace)) {
                            advance();
                            return _ast->addRational(numerator, denominator, p);
                        }
                    }
                }
            }
        }
    }

    _pos = saved;
    NodeID numerator = parseBraceGroup();
    NodeID denominator = parseBraceGroup();
    return _ast->addBinaryOp(BinaryOpKind::Divide, numerator, denominator, p);
}

NodeID Parser::parseSingleArgFunction() {
    const Token& t = advance();
    FunctionKind fKind = FUNCTION_KIND_MAP.at(t.lexeme);
    size_t p = t.pos;

    NodeID arg;
    if (peek().is(TokenType::LBrace)) {
        arg = parseBraceGroup();
    } else if (peek().is(TokenType::LParenthesis)) {
        advance();
        arg = parseExpression(0);
        expect(TokenType::RParenthesis);
    } else {
        arg = parseExpression(PREFIX_UNARY_RBP);
    }

    return _ast->addCall(fKind, {arg}, p); 
}

NodeID Parser::parseOperatorName() {
    size_t p = advance().pos;

    expect(TokenType::LBrace);
    const Token& name = expect(TokenType::Identifier);
    expect(TokenType::RBrace);

    auto it = OPERATOR_NAME_MAP.find(name.lexeme);
    if (it == OPERATOR_NAME_MAP.end()) {
        throw std::runtime_error("unknown operatorname: " + name.lexeme);
    }

    expect(TokenType::LParenthesis);
    std::vector<NodeID> args = parseArgList();
    expect(TokenType::RParenthesis);

    return _ast->addCall(it->second, args, p);
}

std::vector<NodeID> Parser::parseArgList() {
    std::vector<NodeID> args;
    args.push_back(parseExpression(0));
    while (peek().is(TokenType::Comma)) {
        advance();
        args.push_back(parseExpression(0));
    }

    return args;
}

bool Parser::canImplicitMultiply() const {
    const Token& t = peek();
    if (t.is(TokenType::Number)) return true;
    if (t.is(TokenType::Identifier)) return t.lexeme == "!";
    if (t.is(TokenType::LParenthesis)) return true;
    if (t.is(TokenType::LBrace)) return true;
    if (t.is(TokenType::Command)) return PREFIX_COMMANDS.count(t.lexeme) > 0;
    return false;
}

// stern brocot search for a fraction approx of a float
bool Parser::floatToRational(const double& input, RationalNode& output) {
    bool canRationalize = false;
    bool isNegative = false;

    const double maxError = 1e-12;
    const u16 maxDenominator = 1000;

    i64 wholePart = (i64)input;
    double fractionalPart = (double)(input - wholePart);

    i64 numL = 0;   i64 numR = 1;
    i64 denL = 1;   i64 denR = 1;

    while (true) {

        

        if (std::abs(fractionalPart - mediant) <= maxError) {
            output = { numMediant, denMediant, UnknownPos };
            canRationalize = true;
            break;
        }

        i64 numMediant = numL + numR;
        i64 denMediant = denL + denR;
        double mediant = numMediant / denMediant;

        if (denMediant > maxDenominator) break;

        if (mediant < fractionalPart) {
            numL = numMediant;
            denL = denMediant;
        } else if (mediant > fractionalPart) {
            numR = numMediant;
            denR = denMediant;
        }
    }
    
    return canRationalize;
}