#pragma once

#include "Nodes.h"
#include <sstream>
#include <map>
#include <vector>
#include <string>

// not used by anything else except Assembler.cpp
struct Var
{
    std::string name;
    size_t stack_loc;
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
        std::string create_label();
        void begin_scope();
        void end_scope();
        void alignStackAndCall(const std::string &function);

    private:
        const NodeProgram m_program;
        std::stringstream m_output;
        size_t m_stack_size = 0;
        size_t m_label_count = 0;
        std::vector<Var> m_vars{};
        std::vector<size_t> m_scopes{};
    };
}