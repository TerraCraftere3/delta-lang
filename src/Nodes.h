#pragma once

#include "Tokens.h"
#include <variant>
#include <vector>

namespace Delta
{
    struct NodeExpressionIntegerLiteral
    {
        Token int_literal;
        const char *id = "Integer Literal";
    };

    struct NodeExpressionIdentifier
    {
        Token ident;
        const char *id = "Expression Id";
    };

    struct NodeExpression
    {
        std::variant<NodeExpressionIntegerLiteral, NodeExpressionIdentifier> var;
        const char *id = "Expression";
    };

    struct NodeStatementExit
    {
        NodeExpression expression;
        const char *id = "Statement Exit";
    };

    struct NodeStatementLet
    {
        Token ident;
        NodeExpression expression;
        const char *id = "Statement Let";
    };

    struct NodeStatement
    {
        std::variant<NodeStatementExit, NodeStatementLet> var;
        const char *id = "Statement";
    };

    struct NodeProgram
    {
        std::vector<NodeStatement> statements;
        const char *id = "Program";
    };
}