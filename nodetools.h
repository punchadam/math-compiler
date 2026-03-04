#ifndef NODETOOLS_H
#define NODETOOLS_H

#include "ast.h"
#include "Error.h"
#include <set>

bool isConstant(const AST& ast, const NodeID& id) {
    if (id.isNone()) return false;
    const ASTNode::Kind& k = ast.at(id).kind;
    if (std::holds_alternative<ConstantNode>(k)) return true;
    return false;
}

bool isReal(const AST& ast, const NodeID& id) {
    if (id.isNone()) return false;
    const ASTNode::Kind& k = ast.at(id).kind;
    if (std::holds_alternative<RealNode>(k)) return true;
    return false;
}

bool isRational(const AST& ast, const NodeID& id) {
    if (id.isNone()) return false;
    const ASTNode::Kind& k = ast.at(id).kind;
    if (std::holds_alternative<RationalNode>(k)) return true;
    return false;
}

bool isNumeric(const AST& ast, const NodeID& id) {
    return isReal(ast, id) || isRational(ast, id);
}

bool isIdentifier(const AST& ast, const NodeID& id) {
    if (id.isNone()) return false;
    const ASTNode::Kind& k = ast.at(id).kind;
    if (std::holds_alternative<IdentifierNode>(k)) return true;
    return false;
}

bool isBinaryOp(const AST& ast, const NodeID& id) {
    if (id.isNone()) return false;
    const ASTNode::Kind& k = ast.at(id).kind;
    if (std::holds_alternative<BinaryOpNode>(k)) return true;
    return false;
}

bool isUnaryOp(const AST& ast, const NodeID& id) {
    if (id.isNone()) return false;
    const ASTNode::Kind& k = ast.at(id).kind;
    if (std::holds_alternative<UnaryOpNode>(k)) return true;
    return false;
}

bool isCall(const AST& ast, const NodeID& id) {
    if (id.isNone()) return false;
    const ASTNode::Kind& k = ast.at(id).kind;
    if (std::holds_alternative<CallNode>(k)) return true;
    return false;
}

const std::optional<double> getReal(const AST& ast, const NodeID& id) {
    if (!isReal(ast, id)) return std::nullopt;
    return std::get<RealNode>(ast.at(id).kind).value;
}

std::optional<RationalNode> getRational(const AST& ast, const NodeID& id) {
    if (!isRational(ast, id)) return std::nullopt;
    return std::get<RationalNode>(ast.at(id).kind);
}

std::optional<std::string> getIdentifierName(const AST& ast, const NodeID& id) {
    if (!isIdentifier(ast, id)) return std::nullopt;
    return std::get<IdentifierNode>(ast.at(id).kind).name;
}

std::optional<BinaryOpNode> getBinaryOp(const AST& ast, const NodeID& id) {
    if (!isBinaryOp(ast, id)) return std::nullopt;
    return std::get<BinaryOpNode>(ast.at(id).kind);
}

std::optional<UnaryOpNode> getUnaryOp(const AST& ast, const NodeID& id) {
    if (!isUnaryOp(ast, id)) return std::nullopt;
    return std::get<UnaryOpNode>(ast.at(id).kind);
}

std::optional<CallNode> getCall(const AST& ast, const NodeID& id) {
    if (!isCall(ast, id)) return std::nullopt;
    return std::get<CallNode>(ast.at(id).kind);
}

const bool isZero(const AST& ast, const NodeID& id) {
    if (auto r = getReal(ast, id)) return *r == 0.0;
    if (auto r = getRational(ast, id)) return r->numerator == 0;
    return false;
}

const bool isOne(const AST& ast, const NodeID& id) {
    if (auto r = getReal(ast, id)) return *r == 1.0;
    if (auto r = getRational(ast, id)) return r->numerator == r->denominator;
    return false;
}

const bool isNegativeOne(const AST& ast, const NodeID& id) {
    if (auto r = getReal(ast, id)) return *r == -1.0;
    if (auto r = getRational(ast, id)) return -r->numerator == r->denominator;
    return false;
}

const std::optional<double> toDouble(const AST& ast, const NodeID& id) {
    if (auto r = getReal(ast, id)) return *r;
    if (auto r = getRational(ast, id)) return (double)r->numerator / (double)r->denominator;
    return std::nullopt;
}

bool containsIdentifier(const AST& ast, const NodeID& id, const std::string& name) {
    if (id.isNone()) return false;

    if (auto n = getIdentifierName(ast, id)) return *n == name;
    if (auto b = getBinaryOp(ast, id)) return containsIdentifier(ast, b->right, name) || containsIdentifier(ast, b->left, name);
    if (auto u = getUnaryOp(ast, id)) return containsIdentifier(ast, u->inner, name);
    if (auto c = getCall(ast, id)) {
        for (const NodeID& arg : c->args) {
            if (containsIdentifier(ast, arg, name)) return true;
        }
    }

    return false;
}

std::set<std::string> collectIdentifiers(const AST& ast, const NodeID& id) {
    if (id.isNone()) return {};

    if (auto n = getIdentifierName(ast, id)) return { *n };

    if (auto b = getBinaryOp(ast, id)) {
        auto left  = collectIdentifiers(ast, b->left);
        auto right = collectIdentifiers(ast, b->right);
        left.insert(right.begin(), right.end());
        return left;
    }
    if (auto u = getUnaryOp(ast, id)) return collectIdentifiers(ast, u->inner);
    if (auto c = getCall(ast, id)) {
        std::set<std::string> result;
        for (const NodeID& arg : c->args) {
            auto argSet = collectIdentifiers(ast, arg);
            result.insert(argSet.begin(), argSet.end());
        }
        return result;
    }
    return {};
}

NodeID cloneSubtree(const AST& in, const NodeID& id, AST& out) {
    if (id.isNone()) return NodeID::None();

    return std::visit([&](const auto& node) -> NodeID {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, ConstantNode>) {
            return out.addConstant(node.cKind, node.pos);
        } if constexpr (std::is_same_v<T, RealNode>) {
            return out.addReal(node.value, node.pos);
        } if constexpr (std::is_same_v<T, RationalNode>) {
            return out.addRational(node.numerator, node.denominator, node.pos);
        } if constexpr (std::is_same_v<T, IdentifierNode>) {
            return out.addIdentifier(node.name, node.pos);
        } if constexpr (std::is_same_v<T, BinaryOpNode>) {
            return out.addBinaryOp(node.bKind, cloneSubtree(in, node.left, out), cloneSubtree(in, node.right, out), node.pos);
        } if constexpr (std::is_same_v<T, UnaryOpNode>) {
            return out.addUnaryOp(node.uKind, cloneSubtree(in, node.inner, out), node.pos);
        } if constexpr (std::is_same_v<T, CallNode>) {
            std::vector<NodeID> args;
            args.reserve(node.args.size());
            for (const NodeID& arg : node.args) {
                args.push_back(cloneSubtree(in, arg, out));
            }
            return out.addCall(node.fKind, args, node.pos);
        }
    }, in.at(id).kind);
}

#endif