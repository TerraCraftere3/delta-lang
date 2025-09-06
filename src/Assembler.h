#pragma once

#include "Nodes.h"
#include <sstream>
#include <unordered_map>

// not used by anything else
struct Var
{
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
        void generateExpression(const NodeExpression *expression);
        void generateStatement(const NodeStatement *statement);

    private:
        void push(const std::string &reg);
        void pop(const std::string &reg);
        void alignStackAndCall(const std::string &function);

    private:
        const NodeProgram m_program;
        std::stringstream m_output;
        size_t m_stack_size = 0;
        std::unordered_map<std::string, Var> m_vars{};
    };
}