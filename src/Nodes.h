#pragma once

#include "Tokens.h"
#include "Types.h"
#include <variant>
#include <vector>

namespace Delta
{
    struct NodeExpression;
    struct NodeTermDoubleLiteral
    {
        Token double_literal;
        const char *id = "Double Literal";
    };

    struct NodeTermFloatLiteral
    {
        Token float_literal;
        const char *id = "Float Literal";
    };

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

    struct NodeTermParen
    {
        NodeExpression *expr;
        const char *id = "()";
    };

    struct NodeTermCast
    {
        NodeExpression *expr;
        DataType target_type;
        const char *id = "Cast";
    };

    struct NodeTermFunctionCall
    {
        Token function_name;
        std::vector<NodeExpression *> arguments;
        const char *id = "Function Call";
    };

    struct NodeExpressionBinaryGreaterEquals
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Greater Equals";
        const char *binaryName = "Greater or Equals";
    };

    struct NodeExpressionBinaryGreater
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Greater ";
        const char *binaryName = "Greater than";
    };

    struct NodeExpressionBinaryLessEquals
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Less Equals";
        const char *binaryName = "Less or Equals";
    };

    struct NodeExpressionBinaryLess
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Less";
        const char *binaryName = "Less than";
    };

    struct NodeExpressionBinaryEquals
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Equals";
        const char *binaryName = "Equals";
    };

    struct NodeExpressionBinaryAddition
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Addition";
        const char *binaryName = "Addition";
    };

    struct NodeExpressionBinarySubtraction
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Subtraction";
        const char *binaryName = "Subtraction";
    };

    struct NodeExpressionBinaryDivision
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Division";
        const char *binaryName = "Division";
    };

    struct NodeExpressionBinaryMultiplication
    {
        NodeExpression *left;
        NodeExpression *right;
        const char *id = "Binary Expression Multiplication";
        const char *binaryName = "Multiplication";
    };

    struct NodeExpressionBinary
    {
        std::variant<
            NodeExpressionBinaryAddition *,
            NodeExpressionBinarySubtraction *,
            NodeExpressionBinaryMultiplication *,
            NodeExpressionBinaryDivision *,
            NodeExpressionBinaryGreaterEquals *,
            NodeExpressionBinaryGreater *,
            NodeExpressionBinaryLessEquals *,
            NodeExpressionBinaryLess *,
            NodeExpressionBinaryEquals *>
            var;
        const char *id = "Binary Expression";
    };

    struct NodeExpressionTerm
    {
        std::variant<NodeTermIntegerLiteral *, NodeTermFloatLiteral *, NodeTermDoubleLiteral *, NodeTermIdentifier *, NodeTermParen *, NodeTermFunctionCall *, NodeTermCast *> var;
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

    struct NodeStatementReturn
    {
        NodeExpression *expression; // Optional - can be nullptr for void returns
        const char *id = "Statement Return";
    };

    struct NodeParameter
    {
        Token ident;
        DataType type;
        const char *id = "Parameter";
    };

    struct NodeFunctionDeclaration
    {
        Token function_name;
        std::vector<NodeParameter *> parameters;
        DataType return_type;
        NodeScope *body;
        const char *id = "Function Declaration";
    };

    struct NodeStatement
    {
        std::variant<NodeStatementExit *, NodeStatementLet *, NodeStatementAssign *, NodeStatementIf *, NodeScope *, NodeStatementReturn *> var;
        const char *id = "Statement";
    };

    struct NodeProgram
    {
        std::vector<NodeFunctionDeclaration *> functions;
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