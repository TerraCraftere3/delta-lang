#include "Assembler.h"

#include "Log.h"

namespace Delta
{
    Assembler::Assembler(NodeProgram root) : m_program(root) {}

    std::string Assembler::generate()
    {
        m_output << "global _start\n";      // ASM: Text Section
        m_output << "extern ExitProcess\n"; // ASM: WinAPI ExitProcess
        m_output << "\n";                   // ASM:
        m_output << "section .text\n";      // ASM: Entry Point definition
        m_output << "_start:\n";            // ASM: Start Label

        for (const NodeStatement &statement : m_program.statements)
        {
            generateStatement(statement);
        }

        m_output << "\tsub rsp, 40\n";      // ASM: Align Stack
        m_output << "\tmov ecx, 0\n";       // ASM: Move 0 to Exit Code Register (ecx)
        m_output << "\tcall ExitProcess\n"; // ASM: Call ExitProcess
        return m_output.str();
    }

    void Assembler::generateExpression(const NodeExpression &expression)
    {
        struct ExpressionVisitor
        {
            Assembler *generator;
            ExpressionVisitor(Assembler *gen) : generator(gen) {}

            void
            operator()(const NodeExpressionIntegerLiteral &expression_int_lit) const
            {
                generator->m_output << "\tmov rax, " << expression_int_lit.int_literal.value.value() << "\n"; // ASM: Move integer into rax
                generator->push("rax");                                                                       // ASM: Push rax (integer) to the stack
            }
            void operator()(const NodeExpressionIdentifier &expression_identifier) const
            {
                if (!generator->m_vars.contains(expression_identifier.ident.value.value()))
                {
                    LOG_ERROR("Undeclared identifier {}", expression_identifier.ident.value.value());
                    exit(EXIT_FAILURE);
                }
                const auto var = generator->m_vars.at(expression_identifier.ident.value.value());

                size_t offset = (generator->m_stack_size - var.stack_loc - 1) * 8;
                generator->push("QWORD [rsp+" + std::to_string(offset) + "]");
            }
        };

        ExpressionVisitor visitor(this);
        std::visit(visitor, expression.var);
    };

    void Assembler::generateStatement(const NodeStatement &statement)
    {
        struct StatementVisitor
        {
            Assembler *generator;
            StatementVisitor(Assembler *gen) : generator(gen) {}

            void operator()(const NodeStatementExit &statement_exit)
            {
                generator->generateExpression(statement_exit.expression); // ASM: Expression
                generator->pop("rcx");                                    // ASM: Pop into Exit Code Register (rcx)
                generator->m_output << "\tsub rsp, 40\n";                 // ASM: Align Stack
                generator->m_output << "\tcall ExitProcess\n";            // ASM: Call ExitProcess
            }
            void operator()(const NodeStatementLet &statement_let)
            {
                if (generator->m_vars.contains(statement_let.ident.value.value()))
                {
                    LOG_ERROR("Identifier '{}' exists already", statement_let.ident.value.value());
                    exit(EXIT_FAILURE);
                }
                generator->m_vars.insert({statement_let.ident.value.value(), Var{generator->m_stack_size}});
                generator->generateExpression(statement_let.expression);
            }
        };

        StatementVisitor visitor(this);
        std::visit(visitor, statement.var);
    }

    void Assembler::push(const std::string &reg)
    {
        m_output << "\tpush " << reg << " ; Stack now at " << ++m_stack_size << "\n";
    }
    void Assembler::pop(const std::string &reg)
    {
        m_output << "\tpop " << reg << " ; Stack now at " << --m_stack_size << "\n";
    }
} // namespace Delta