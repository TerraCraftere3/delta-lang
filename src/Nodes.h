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
#ifdef DELTA_NODE_ID
        const char *id = "Double Literal";
#endif
    };

    struct NodeTermFloatLiteral
    {
        Token float_literal;
#ifdef DELTA_NODE_ID
        const char *id = "Float Literal";
#endif
    };

    struct NodeTermIntegerLiteral
    {
        Token int_literal;
#ifdef DELTA_NODE_ID
        const char *id = "Integer Literal";
#endif
    };

    struct NodeTermStringLiteral
    {
        Token string_literal;
#ifdef DELTA_NODE_ID
        const char *id = "String Literal";
#endif
    };

    struct NodeTermIdentifier
    {
        Token ident;
#ifdef DELTA_NODE_ID
        const char *id = "Expression Identifier";
#endif
    };

    struct NodeTermParen
    {
        NodeExpression *expr;
#ifdef DELTA_NODE_ID
        const char *id = "()";
#endif
    };

    struct NodeTermCast
    {
        NodeExpression *expr;
        DataType target_type;
#ifdef DELTA_NODE_ID
        const char *id = "Cast";
#endif
    };

    struct NodeTermAddressOf
    {
        Token ident;
#ifdef DELTA_NODE_ID
        const char *id = "Address Of";
#endif
    };

    struct NodeTermDereference
    {
        NodeExpression *expr;
#ifdef DELTA_NODE_ID
        const char *id = "Dereference";
#endif
    };

    struct NodeTermArrayAccess
    {
        NodeExpression *array_expr;
        NodeExpression *index_expr;
#ifdef DELTA_NODE_ID
        const char *id = "Array Access";
#endif
    };

    struct NodeTermFunctionCall
    {
        Token function_name;
        std::vector<NodeExpression *> arguments;
#ifdef DELTA_NODE_ID
        const char *id = "Function Call";
#endif
    };

    struct NodeExpressionBinaryGreaterEquals
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Greater Equals";
#endif
        const char *binaryName = "Greater or Equals";
    };

    struct NodeExpressionBinaryGreater
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Greater ";
#endif
        const char *binaryName = "Greater than";
    };

    struct NodeExpressionBinaryLessEquals
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Less Equals";
#endif
        const char *binaryName = "Less or Equals";
    };

    struct NodeExpressionBinaryLess
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Less";
#endif
        const char *binaryName = "Less than";
    };

    struct NodeExpressionBinaryEquals
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Equals";
#endif
        const char *binaryName = "Equals";
    };

    struct NodeExpressionBinaryAddition
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Addition";
#endif
        const char *binaryName = "Addition";
    };

    struct NodeExpressionBinarySubtraction
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Subtraction";
#endif
        const char *binaryName = "Subtraction";
    };

    struct NodeExpressionBinaryDivision
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Division";
#endif
        const char *binaryName = "Division";
    };

    struct NodeExpressionBinaryMultiplication
    {
        NodeExpression *left;
        NodeExpression *right;
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression Multiplication";
#endif
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
#ifdef DELTA_NODE_ID
        const char *id = "Binary Expression";
#endif
    };

    struct NodeExpressionTerm
    {
        std::variant<
            NodeTermIntegerLiteral *,
            NodeTermFloatLiteral *,
            NodeTermDoubleLiteral *,
            NodeTermStringLiteral *,
            NodeTermIdentifier *,
            NodeTermParen *,
            NodeTermFunctionCall *,
            NodeTermCast *,
            NodeTermAddressOf *,
            NodeTermDereference *,
            NodeTermArrayAccess *>
            var;
#ifdef DELTA_NODE_ID
        const char *id = "Term Expression";
#endif
    };

    struct NodeExpression
    {
        std::variant<NodeExpressionTerm *, NodeExpressionBinary *> var;
#ifdef DELTA_NODE_ID
        const char *id = "Expression";
#endif
    };

    struct NodeStatement;

    struct NodeScope
    {
        std::vector<NodeStatement *> statements;
#ifdef DELTA_NODE_ID
        const char *id = "Scope";
#endif
    };

    struct NodeIfPred;

    struct NodeIfPredElif
    {
        NodeExpression *expr;
        NodeScope *scope;
        std::optional<NodeIfPred *> pred;
#ifdef DELTA_NODE_ID
        const char *id = "If Pred Elif";
#endif
    };

    struct NodeIfPredElse
    {
        NodeScope *scope;
#ifdef DELTA_NODE_ID
        const char *id = "If Pred Else";
#endif
    };

    struct NodeIfPred
    {
        std::variant<NodeIfPredElif *, NodeIfPredElse *> var;
#ifdef DELTA_NODE_ID
        const char *id = "If Pred";
#endif
    };

    struct NodeStatementIf
    {
        NodeExpression *expr;
        NodeScope *scope;
        std::optional<NodeIfPred *> pred;
#ifdef DELTA_NODE_ID
        const char *id = "Statement If";
#endif
    };

    struct NodeStatementExit
    {
        NodeExpression *expression;
#ifdef DELTA_NODE_ID
        const char *id = "Statement Exit";
#endif
    };

    struct NodeStatementAssign
    {
        Token ident;
        NodeExpression *expression;
#ifdef DELTA_NODE_ID
        const char *id = "Statement Assign";
#endif
    };

    struct NodeStatementLet
    {
        Token ident;
        NodeExpression *expression;
        DataType type;
        bool isConst = false;
#ifdef DELTA_NODE_ID
        const char *id = "Statement Let";
#endif
    };

    struct NodeStatementReturn
    {
        NodeExpression *expression; // Optional - can be nullptr for void returns
#ifdef DELTA_NODE_ID
        const char *id = "Statement Return";
#endif
    };

    struct NodeStatementPointerAssign
    {
        NodeExpression *ptr_expr;
        NodeExpression *value_expr;
#ifdef DELTA_NODE_ID
        const char *id = "Pointer Assign";
#endif
    };

    struct NodeStatementArrayAssign
    {
        NodeExpression *array_expr;
        NodeExpression *index_expr;
        NodeExpression *value_expr;
#ifdef DELTA_NODE_ID
        const char *id = "Array Assign";
#endif
    };

    struct NodeStatementWhile
    {
        NodeExpression *expr;
        NodeScope *scope;
#ifdef DELTA_NODE_ID
        const char *id = "While Loop";
#endif
    };

    struct NodeParameter
    {
        Token ident;
        DataType type;
#ifdef DELTA_NODE_ID
        const char *id = "Parameter";
#endif
    };

    struct NodeFunctionDeclaration
    {
        Token function_name;
        std::vector<NodeParameter *> parameters;
        DataType return_type;
        NodeScope *body;
#ifdef DELTA_NODE_ID
        const char *id = "Function Declaration";
#endif
    };

    struct NodeExternalDeclaration
    {
        Token function_name;
        std::vector<DataType> parameters;
        DataType return_type;
        bool is_variadic; // any amount of variables, like printf(str, ...)
#ifdef DELTA_NODE_ID
        const char *id = "External Declaration";
#endif
    };

    struct NodeStatement
    {
        std::variant<
            NodeStatementExit *,
            NodeStatementLet *,
            NodeStatementAssign *,
            NodeStatementIf *,
            NodeStatementWhile *,
            NodeScope *,
            NodeStatementReturn *,
            NodeExpression *,
            NodeStatementPointerAssign *,
            NodeStatementArrayAssign *>
            var;
#ifdef DELTA_NODE_ID
        const char *id = "Statement";
#endif
    };

    struct NodeProgram
    {
        std::vector<NodeExternalDeclaration *> externals;
        std::vector<NodeFunctionDeclaration *> functions;
        std::vector<NodeStatement *> statements;
#ifdef DELTA_NODE_ID
        const char *id = "Program";
#endif
    };

    template <typename T>
    std::string getNodeID(T node)
    {
        std::string name = std::string(node.id);
        return name;
    }
}