#pragma once

#include "Tokens.h"
#include "Types.h"
#include <variant>
#include <vector>

namespace Delta
{
    struct NodeTermIntegerLiteral
    {
        Token int_literal;
        const char *id = "Integer Literal";
    };

    struct NodeTermIdentifier
    {
        Token ident;
        const char *id = "Expression Identifier";
    };

    struct NodeExpression;

    struct NodeTermParen
    {
        NodeExpression *expr;
    };

    struct NodeExpressionBinaryAddition
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Addition";
    };

    struct NodeExpressionBinarySubtraction
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Subtraction";
    };

    struct NodeExpressionBinaryDivision
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Division";
    };

    struct NodeExpressionBinaryMultiplication
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Multiplication";
    };

    struct NodeExpressionBinary
    {
        std::variant<NodeExpressionBinaryAddition *, NodeExpressionBinarySubtraction *, NodeExpressionBinaryMultiplication *, NodeExpressionBinaryDivision *> var;
        const char *id = "Binary Expression";
    };

    struct NodeExpressionTerm
    {
        std::variant<NodeTermIntegerLiteral *, NodeTermIdentifier *, NodeTermParen *> var;
        const char *id = "Term Expression";
    };

    struct NodeExpression
    {
        std::variant<NodeExpressionTerm *, NodeExpressionBinary *> var;
        const char *id = "Expression";
    };

    struct NodeStatement;

    struct NodeScope
    {
        std::vector<NodeStatement *> statements;
        const char *id = "Scope";
    };

    struct NodeIfPred;

    struct NodeIfPredElif
    {
        NodeExpression *expr;
        NodeScope *scope;
        std::optional<NodeIfPred *> pred;
        const char *id = "If Pred Elif";
    };

    struct NodeIfPredElse
    {
        NodeScope *scope;
        const char *id = "If Pred Else";
    };

    struct NodeIfPred
    {
        std::variant<NodeIfPredElif *, NodeIfPredElse *> var;
        const char *id = "If Pred";
    };

    struct NodeStatementIf
    {
        NodeExpression *expr;
        NodeScope *scope;
        std::optional<NodeIfPred *> pred;
        const char *id = "Statement If";
    };

    struct NodeStatementExit
    {
        NodeExpression *expression;
        const char *id = "Statement Exit";
    };

    struct NodeStatementAssign
    {
        Token ident;
        NodeExpression *expression;
        const char *id = "Statement Assign";
    };

    struct NodeStatementLet
    {
        Token ident;
        NodeExpression *expression;
        DataType type;
        bool isConst = false;
        const char *id = "Statement Let";
    };

    struct NodeStatement
    {
        std::variant<NodeStatementExit *, NodeStatementLet *, NodeStatementAssign *, NodeStatementIf *, NodeScope *> var;
        const char *id = "Statement";
    };

    struct NodeProgram
    {
        std::vector<NodeStatement *> statements;
        const char *id = "Program";
    };

    template <typename T>
    std::string getNodeID(T node)
    {
        std::string name = std::string(node.id);
        return name;
    }
}