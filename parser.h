#ifndef PARSER_H
#define PARSER_H

/*
Pratt Parsing

    pls read https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html

In short, we need to take a linear token stream
and turn it into an AST (Abstract Syntax TREE).
This requires a technique called recursive descent.

    Recursivce Descent Parsing: The parser starts with the grammar's
    start symbol (the root of the parse tree) and tries to build the
    parse tree downwards to match the input tokens.

Pratt parsing combines all rules into one function,
called parseExpression(minBindingPower), that uses
binding power to decide when to stop consuming tokens.

    Binding power: every operator has L and R binding power, which
    encodes both its precedence in order of operations AND its
    associativity (L or R). Consider "1 + 2 * 3". The '+' has a 
    lower binding power than '*', so when parsing the right side
    of '+', encountering '*' steals that 2 because it binds tighter.

Using asymmetric bindings means L and R associativity
is automatically encoded. When two ops are next to each
other, the side with the lower number loses the tug of
war, so the parse tree groups that direction.

    "Not only does it send your mind into Möbeus-shaped hamster wheel,
    it also handles associativity and precedence!"  - matklad

--
There are 3 types of token in Pratt parsing:

    1. Prefix (some call it "nud" for null denotation)
        Tokens that start an expression (numbers, identifiers, '(',
        unary '-', functions like "sin(", etc).
    2. Infix (aka "led" for left denotation)
        Tokens that appear between expressions ('+', '*', '^', etc).
    3. Postfix
        Tokens that appear after an expression ('!', '%').

The core loop:

    parseExpression(minBindingPower) :
        left_side = parsePrefix()     // consume one prefix thing
        loop:
            op = peek next token
            if op is postfix anmd its left_bp >= minBindingPower:
                consume it
                wrap left_side
                continue
            if op is infix and its left_bp >= minBindingPower:
                consume it
                right_side = parseExpression(right_bp)  // recurse
                left_side = makeBinaryNode(op, left_side, right_side)
                continue
            break
        return left_side

Recursion with right_bp is what creates the tree structure.
It determines how much of the remaining tokens the right
side can consume.
*/


#include "AST.h"
#include "lexer.h"



#endif