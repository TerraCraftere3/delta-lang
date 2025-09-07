#pragma once

#include "Nodes.h"
#include "Types.h"
#include <sstream>
#include <map>
#include <vector>
#include <string>

// not used by anything else except Assembler.cpp
struct Var
{
    std::string name;
    size_t stack_loc;
    Delta::DataType type;
    size_t type_size;

    Var(const std::string &n, size_t loc, Delta::DataType t)
        : name(n), stack_loc(loc), type(t), type_size(Delta::getTypeSize(t)) {}
};

namespace Delta
{
    class Assembler
    {
    public:
        Assembler(NodeProgram root);
        std::string generate();
        void generateTerm(const NodeExpressionTerm *term);
        void generateBinaryExpression(const NodeExpressionBinary *bin_expr);
        void generateExpression(const NodeExpression *expression);
        void generateScope(const NodeScope *scope);
        void generateIfPred(const NodeIfPred *pred, const std::string &end_label);
        void generateStatement(const NodeStatement *statement);

    private:
        void push(const std::string &reg);
        void pop(const std::string &reg);
        void pushTyped(const std::string &reg, DataType type);
        void popTyped(const std::string &reg, DataType type);
        std::string create_label();
        void begin_scope();
        void end_scope();
        void alignStackAndCall(const std::string &function);

        // Type-aware helper methods
        std::string getAppropriateRegister(DataType type, const std::string &base_reg = "rax");
        void generateTypedMove(const std::string &dest, const std::string &src, DataType type);
        void generateTypedBinaryOp(const std::string &op, DataType leftType, DataType rightType);
        DataType inferExpressionType(const NodeExpression *expression);
        DataType inferTermType(const NodeExpressionTerm *term);
        DataType inferBinaryExpressionType(const NodeExpressionBinary *bin_expr);
        void validateTypeCompatibility(DataType expected, DataType actual, const std::string &context);

    private:
        const NodeProgram m_program;
        std::stringstream m_output;
        size_t m_stack_size = 0;
        size_t m_stack_byte_size = 0;
        size_t m_label_count = 0;
        std::vector<Var> m_vars{};
        std::vector<size_t> m_scopes{};

        // Type tracking for expressions
        std::map<const NodeExpression *, DataType> m_expression_types{};
    };
}