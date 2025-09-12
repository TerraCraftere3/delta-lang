#include "Debug.h"
#include "Strings.h"

namespace Delta
{
    std::string nodeDebugPrint(NodeStatementReturn *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Return\n";
        output << nodeDebugPrint(node->expression, indention + 1);
        return output.str();
    }

    std::string nodeDebugPrint(NodeStatementIf *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "If\n";

        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Expression\n";
        output << nodeDebugPrint(node->expr, indention + 2);

        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Scope\n";
        output << nodeDebugPrint(node->scope, indention + 2);

        if (node->pred.has_value())
            output << nodeDebugPrint(node->pred.value(), indention);
        return output.str();
    }

    std::string nodeDebugPrint(NodeStatementWhile *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "While\n";

        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Expression\n";
        output << nodeDebugPrint(node->expr, indention + 2);

        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Scope\n";
        output << nodeDebugPrint(node->scope, indention + 2);

        return output.str();
    }

    std::string nodeDebugPrint(NodeIfPred *node, int indention)
    {
        std::string output = std::visit([&](auto &obj)
                                        { return nodeDebugPrint(obj, indention); }, node->var);
        return output;
    }

    std::string nodeDebugPrint(NodeIfPredElif *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Elif\n";

        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Expression\n";
        output << nodeDebugPrint(node->expr, indention + 2);

        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Scope\n";
        output << nodeDebugPrint(node->scope, indention + 2);

        if (node->pred.has_value())
            output << nodeDebugPrint(node->pred.value(), indention);
        return output.str();
    }

    std::string nodeDebugPrint(NodeIfPredElse *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Else\n";

        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Scope\n";
        output << nodeDebugPrint(node->scope, indention + 2);
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermDereference *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Dereference\n";
        output << nodeDebugPrint(node->expr, indention + 1);
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermAddressOf *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Address of \""
               << node->ident.value.value() << "\"\n";
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermParen *node, int indention)
    {
        std::stringstream output;
        output << nodeDebugPrint(node->expr, indention);
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermFunctionCall *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Call \"";
        output << node->function_name.value.value();
        output << "\"\n";
        for (auto expression : node->arguments)
        {
            output << nodeDebugPrint(expression, indention + 1);
        }
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermCast *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Cast ";
        output << typeToString(node->target_type);
        output << "\n";
        output << nodeDebugPrint(node->expr, indention + 1);
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermIntegerLiteral *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Literal: " << node->int_literal.value.value() << "\n";
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermStringLiteral *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Literal: \"" << escape(node->string_literal.value.value()) << "\"\n";
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermFloatLiteral *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Literal: " << node->float_literal.value.value() << "f\n";
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermDoubleLiteral *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Literal: " << node->double_literal.value.value() << "\n";
        return output.str();
    }

    std::string nodeDebugPrint(NodeTermIdentifier *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Variable \"" << node->ident.value.value() << "\"\n";
        return output.str();
    }

    std::string nodeDebugPrint(NodeExpressionBinary *node, int indention)
    {
        std::string output = std::visit([&](auto &obj)
                                        { return nodeDebugPrint(obj, indention); }, node->var);
        return output;
    }

    std::string nodeDebugPrint(NodeExpressionTerm *node, int indention)
    {
        std::string output = std::visit([&](auto &obj)
                                        { return nodeDebugPrint(obj, indention); }, node->var);
        return output;
    }

    std::string nodeDebugPrint(NodeExpression *node, int indention)
    {
        std::string output = std::visit([&](auto &obj)
                                        { return nodeDebugPrint(obj, indention); }, node->var);
        return output;
    }

    std::string nodeDebugPrint(NodeStatementArrayAssign *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Pointer Assign\n";
        output << Indent(indention + 1) << DEBUG_NODE_PREFIX
               << "Array Expression\n";
        output << nodeDebugPrint(node->array_expr, indention + 2);
        output << Indent(indention + 1) << DEBUG_NODE_PREFIX
               << "Index Expression\n";
        output << nodeDebugPrint(node->index_expr, indention + 2);
        output << Indent(indention + 1) << DEBUG_NODE_PREFIX
               << "Value Expression\n";
        output << nodeDebugPrint(node->value_expr, indention + 2);
        return output.str();
    }

    std::string nodeDebugPrint(NodeStatementPointerAssign *node,
                               int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Pointer Assign\n";
        output << Indent(indention + 1) << DEBUG_NODE_PREFIX
               << "Pointer Expression\n";
        output << nodeDebugPrint(node->ptr_expr, indention + 2);
        output << Indent(indention + 1) << DEBUG_NODE_PREFIX
               << "Value Expression\n";
        output << nodeDebugPrint(node->value_expr, indention + 2);
        return output.str();
    }

    std::string nodeDebugPrint(NodeStatementExit *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Exit\n";
        output << nodeDebugPrint(node->expression, indention + 1);
        return output.str();
    }

    std::string nodeDebugPrint(NodeStatementLet *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX;
        output << "Let ";
        if (node->isConst)
            output << "const ";
        output << "\"" << typeToString(node->type) << " " << node->ident.value.value() << "\"";
        output << "\n";

        output << nodeDebugPrint(node->expression, indention + 1);
        return output.str();
    }

    std::string nodeDebugPrint(NodeStatementAssign *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Assign \"" << node->ident.value.value() << "\"\n";
        output << nodeDebugPrint(node->expression, indention + 1);
        return output.str();
    }

    std::string nodeDebugPrint(NodeStatement *node, int indention)
    {
        std::string output = std::visit([&](auto &obj)
                                        { return nodeDebugPrint(obj, indention); }, node->var);
        return output;
    }

    std::string nodeDebugPrint(NodeScope *node, int indention)
    {
        std::stringstream output;
        for (auto statements : node->statements)
        {
            output << nodeDebugPrint(statements, indention);
        }
        return output.str();
    }

    std::string nodeDebugPrint(NodeFunctionDeclaration *node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX;
        output << "Define \"" << typeToString(node->return_type) << " " << node->function_name.value.value();
        output << "(";
        bool firstArg = true;
        for (auto argument : node->parameters)
        {
            if (!firstArg)
                output << ", ";
            output << typeToString(argument->type) << " " << argument->ident.value.value();
            firstArg = false;
        }
        output << ")\"\n";
        output << nodeDebugPrint(node->body, indention + 1);
        return output.str();
    }

    std::string nodeDebugPrint(NodeProgram node, int indention)
    {
        std::stringstream output;
        output << Indent(indention) << DEBUG_NODE_PREFIX << "Node Program\n";
        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Functions\n";
        for (auto function : node.functions)
        {
            output << nodeDebugPrint(function, indention + 2);
        }
        output << Indent(indention + 1) << DEBUG_NODE_PREFIX << "Statements\n";
        for (auto statements : node.statements)
        {
            output << nodeDebugPrint(statements, indention + 2);
        }

        return output.str();
    }

    std::string Indent(int level)
    {
        return std::string(level, '\t');
    }
} // namespace Delta