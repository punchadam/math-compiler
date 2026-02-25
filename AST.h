#ifndef AST_H
#define AST_H

#include "lookupstuff.h"
#include <variant>

struct NodeID {
    size_t i = 0;
    static constexpr size_t noPosition = (size_t)-1;
    static NodeID None() { return NodeID{ noPosition }; }
    bool isNone() const { return i == noPosition; }
};

enum class ConstantKind {
    PI,
    E,
    I
};
struct ConstantNode {
    ConstantKind cKind;
    size_t pos = 0;
};

struct RealNode {
    double value = 0.0;
    size_t pos = 0;
};

struct RationalNode {
    i64 numerator = 0;
    i64 denominator = 0;
    size_t pos = 0;
};

struct IdentifierNode {
    std::string name;
    size_t pos = 0;
};

struct BinaryOpNode {
    std::string op;
    NodeID left = NodeID::None();
    NodeID right = NodeID::None();
    size_t pos = 0;
};

struct UnaryOpNode {
    std::string op;
    NodeID inner = NodeID::None();
    size_t pos = 0;
};

enum class FunctionKind {
    Sine,
    Cosine,
    Tangent,
    Atan2,

    AbsoluteValue,
    Exponential,
    NaturalLogarithm,
    Logarithm,
    Hypotenuse,
    Max,
    Min
};
struct CallNode {
    FunctionKind fKind;
    std::vector<NodeID> args;
    size_t pos = 0;
};

struct ASTNode {
    using Kind = std::variant<
    ConstantNode,
    RealNode,
    RationalNode,
    IdentifierNode,
    BinaryOpNode,
    UnaryOpNode,
    CallNode
    >;

    Kind kind;

    template <class T>
    ASTNode(T t) : kind(std::move(t)) {}
};

#endif