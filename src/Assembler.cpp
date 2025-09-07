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

        for (const NodeStatement *statement : m_program.statements)
        {
            generateStatement(statement);
        }

        m_output << "\tmov rcx, 0\n"; // ASM: Move 0 to Exit Code Register (rcx)
        alignStackAndCall("ExitProcess");
        return m_output.str();
    }

    void Assembler::generateTerm(const NodeExpressionTerm *term)
    {
        struct TermVisitor
        {
            Assembler *generator;
            TermVisitor(Assembler *gen) : generator(gen) {}
            void operator()(const NodeTermIntegerLiteral *term_int_lit) const
            {
                generator->m_output << "\tmov rax, " << term_int_lit->int_literal.value.value() << "\n"; // ASM: Move integer into rax
                generator->push("rax");                                                                  // ASM: Push rax (integer) to the stack
            }
            void operator()(const NodeTermIdentifier *term_ident) const
            {
                if (!generator->m_vars.contains(term_ident->ident.value.value()))
                {
                    LOG_ERROR("Undeclared identifier {}", term_ident->ident.value.value());
                    exit(EXIT_FAILURE);
                }
                const auto var = generator->m_vars.at(term_ident->ident.value.value());

                size_t offset = (generator->m_stack_size - var.stack_loc - 1) * 8;
                generator->push("QWORD [rsp+" + std::to_string(offset) + "]");
            }
            void operator()(const NodeTermParen *term_paren) const
            {
                generator->generateExpression(term_paren->expr);
            }
        };
        TermVisitor visitor(this);
        std::visit(visitor, term->var);
    }

    void Assembler::generateBinaryExpression(const NodeExpressionBinary *bin_expr)
    {
        struct BinaryExpressionVisitor
        {
            Assembler *generator;
            BinaryExpressionVisitor(Assembler *gen) : generator(gen) {}

            void operator()(const NodeExpressionBinarySubtraction *sub) const
            {
                generator->generateExpression(sub->right);
                generator->generateExpression(sub->left);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_output << "\tsub rax, rbx\n";
                generator->push("rax");
            }
            void operator()(const NodeExpressionBinaryAddition *add) const
            {
                generator->generateExpression(add->right);
                generator->generateExpression(add->left);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_output << "\tadd rax, rbx\n";
                generator->push("rax");
            }
            void operator()(const NodeExpressionBinaryMultiplication *mul) const
            {
                generator->generateExpression(mul->right);
                generator->generateExpression(mul->left);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_output << "\tmul rbx\n";
                generator->push("rax");
            }
            void operator()(const NodeExpressionBinaryDivision *div) const
            {
                generator->generateExpression(div->right);
                generator->generateExpression(div->left);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_output << "\tdiv rbx\n";
                generator->push("rax");
            }
        };

        BinaryExpressionVisitor visitor(this);
        std::visit(visitor, bin_expr->var);
    }

    void Assembler::generateExpression(const NodeExpression *expression)
    {
        struct ExpressionVisitor
        {
            Assembler *generator;
            ExpressionVisitor(Assembler *gen) : generator(gen) {}

            void
            operator()(const NodeExpressionTerm *expression_term) const
            {
                generator->generateTerm(expression_term);
            }
            void operator()(const NodeExpressionBinary *expression_binary) const
            {
                generator->generateBinaryExpression(expression_binary);
            }
        };

        ExpressionVisitor visitor(this);
        std::visit(visitor, expression->var);
    };

    void Assembler::generateStatement(const NodeStatement *statement)
    {
        struct StatementVisitor
        {
            Assembler *generator;
            StatementVisitor(Assembler *gen) : generator(gen) {}

            void operator()(const NodeStatementExit *statement_exit)
            {
                generator->generateExpression(statement_exit->expression); // ASM: Expression
                generator->pop("rcx");                                     // ASM: Pop into Exit Code Register (rcx)
                generator->alignStackAndCall("ExitProcess");               // ASM: Call ExitProcess
            }
            void operator()(const NodeStatementLet *statement_let)
            {
                if (generator->m_vars.contains(statement_let->ident.value.value()))
                {
                    LOG_ERROR("Identifier '{}' exists already", statement_let->ident.value.value());
                    exit(EXIT_FAILURE);
                }
                generator->m_vars.insert({statement_let->ident.value.value(), Var{generator->m_stack_size}});
                generator->generateExpression(statement_let->expression);
            }
        };

        StatementVisitor visitor(this);
        std::visit(visitor, statement->var);
    }

    void Assembler::alignStackAndCall(const std::string &function)
    {
        size_t current_stack_bytes = m_stack_size * 8;
        size_t misalignment = current_stack_bytes % 16;

        if (misalignment != 8)
        {
            // RSP = (16n + 8)
            size_t needed_adjustment = (misalignment == 0) ? 8 : (16 - misalignment + 8);
            m_output << "\tsub rsp, " << needed_adjustment << " ; Align stack for Windows ABI\n";
            m_output << "\tcall " << function << "\n";
            m_output << "\tadd rsp, " << needed_adjustment << " ; Restore stack after call\n";
        }
        else
        {
            m_output << "\tcall " << function << "\n";
        }
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