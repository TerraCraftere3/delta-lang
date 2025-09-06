#pragma once

#include "Tokens.h"
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

    struct NodeExpressionBinaryAddition
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Addition";
    };

    /*struct NodeExpressionBinaryMultiplication
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Multiplication";
    };*/

    struct NodeExpressionBinary
    {
        NodeExpressionBinaryAddition *add;
        const char *id = "Binary Expression";
    };

    struct NodeExpressionTerm
    {
        std::variant<NodeTermIntegerLiteral *, NodeTermIdentifier *> var;
        const char *id = "Term Expression";
    };

    struct NodeExpression
    {
        std::variant<NodeExpressionTerm *, NodeExpressionBinary *> var;
        const char *id = "Expression";
    };

    struct NodeStatementExit
    {
        NodeExpression *expression;
        const char *id = "Statement Exit";
    };

    struct NodeStatementLet
    {
        Token ident;
        NodeExpression *expression;
        const char *id = "Statement Let";
    };

    struct NodeStatement
    {
        std::variant<NodeStatementExit *, NodeStatementLet *> var;
        const char *id = "Statement";
    };

    struct NodeProgram
    {
        std::vector<NodeStatement *> statements;
        const char *id = "Program";
    };
}