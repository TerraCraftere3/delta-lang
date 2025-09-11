#pragma once

#include "Nodes.h"
#include <string>
#include <sstream>
#include <typeinfo>
#include <cassert>
#include <any>

#define DEBUG_NODE_PREFIX "- "

namespace Delta
{
    std::string Indent(int level);

    std::string nodeDebugPrint(NodeIfPred *node, int indention);
    std::string nodeDebugPrint(NodeIfPredElif *node, int indention);
    std::string nodeDebugPrint(NodeIfPredElse *node, int indention);
    std::string nodeDebugPrint(NodeTermParen *node, int indention);
    std::string nodeDebugPrint(NodeTermFunctionCall *node, int indention);
    std::string nodeDebugPrint(NodeTermCast *node, int indention);
    std::string nodeDebugPrint(NodeTermIntegerLiteral *node, int indention);
    std::string nodeDebugPrint(NodeTermFloatLiteral *node, int indention);
    std::string nodeDebugPrint(NodeTermDoubleLiteral *node, int indention);
    std::string nodeDebugPrint(NodeTermIdentifier *node, int indention);
    std::string nodeDebugPrint(NodeExpressionBinary *node, int indention);
    std::string nodeDebugPrint(NodeExpressionTerm *node, int indention);
    std::string nodeDebugPrint(NodeExpression *node, int indention);
    std::string nodeDebugPrint(NodeStatementExit *node, int indention);
    std::string nodeDebugPrint(NodeStatementLet *node, int indention);
    std::string nodeDebugPrint(NodeStatementAssign *node, int indention);
    std::string nodeDebugPrint(NodeStatementReturn *node, int indention);
    std::string nodeDebugPrint(NodeStatementIf *node, int indention);
    std::string nodeDebugPrint(NodeStatement *node, int indention);
    std::string nodeDebugPrint(NodeScope *node, int indention);
    std::string nodeDebugPrint(NodeFunctionDeclaration *node, int indention);
    std::string nodeDebugPrint(NodeProgram node, int indention = 0);

    template <typename T>
        requires(!std::is_pointer_v<T> || (!std::is_same_v<T, NodeStatementIf *> && !std::is_same_v<T, NodeStatement *> && !std::is_same_v<T, NodeScope *> && !std::is_same_v<T, NodeFunctionDeclaration *> && !std::is_same_v<T, NodeProgram>))
    std::string nodeDebugPrint(const T &node, int indention = 0)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "[" << typeid(T).name() << "] NODE NOT IMPLEMENTED\n";
#ifdef DEBUG_NODE_NOT_IMPLEMENTED_ASSERT
        assert(false && "Debug Node not implemented!!!");
#endif
        return output.str();
    }

    template <typename T>
    struct is_allowed_binary : std::false_type
    {
    };

    template <>
    struct is_allowed_binary<NodeExpressionBinaryGreaterEquals> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinaryLessEquals> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinaryGreater> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinaryLess> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinaryEquals> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinaryAddition> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinarySubtraction> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinaryMultiplication> : std::true_type
    {
    };
    template <>
    struct is_allowed_binary<NodeExpressionBinaryDivision> : std::true_type
    {
    };

    template <typename T>
    std::enable_if_t<is_allowed_binary<T>::value, std::string>
    nodeDebugPrint(T *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << node->binaryName << "\n";
        output << nodeDebugPrint(node->left, indention + 1);
        output << nodeDebugPrint(node->right, indention + 1);
        return output.str();
    }
} // namespace Delta