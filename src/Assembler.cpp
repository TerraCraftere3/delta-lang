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
            Assembler *gen;
            TermVisitor(Assembler *gen) : gen(gen) {}
            void operator()(const NodeTermIntegerLiteral *term_int_lit) const
            {
                gen->m_output << "\tmov rax, " << term_int_lit->int_literal.value.value() << "\n"; // ASM: Move integer into rax
                gen->push("rax");                                                                  // ASM: Push rax (integer) to the stack
            }
            void operator()(const NodeTermIdentifier *term_ident) const
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == term_ident->ident.value.value(); });
                if (it == gen->m_vars.cend())
                {
                    LOG_ERROR("Undeclared identifier {}", term_ident->ident.value.value());
                    exit(EXIT_FAILURE);
                }

                size_t offset = (gen->m_stack_size - (*it).stack_loc - 1) * 8;
                gen->push("QWORD [rsp+" + std::to_string(offset) + "]");
            }
            void operator()(const NodeTermParen *term_paren) const
            {
                gen->generateExpression(term_paren->expr);
            }
        };
        TermVisitor visitor(this);
        std::visit(visitor, term->var);
    }

    void Assembler::generateBinaryExpression(const NodeExpressionBinary *bin_expr)
    {
        struct BinaryExpressionVisitor
        {
            Assembler *gen;
            BinaryExpressionVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeExpressionBinarySubtraction *sub) const
            {
                gen->generateExpression(sub->right);
                gen->generateExpression(sub->left);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "\tsub rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeExpressionBinaryAddition *add) const
            {
                gen->generateExpression(add->right);
                gen->generateExpression(add->left);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "\tadd rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeExpressionBinaryMultiplication *mul) const
            {
                gen->generateExpression(mul->right);
                gen->generateExpression(mul->left);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "\tmul rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeExpressionBinaryDivision *div) const
            {
                gen->generateExpression(div->right);
                gen->generateExpression(div->left);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "\tdiv rbx\n";
                gen->push("rax");
            }
        };

        BinaryExpressionVisitor visitor(this);
        std::visit(visitor, bin_expr->var);
    }

    void Assembler::generateExpression(const NodeExpression *expression)
    {
        struct ExpressionVisitor
        {
            Assembler *gen;
            ExpressionVisitor(Assembler *gen) : gen(gen) {}

            void
            operator()(const NodeExpressionTerm *expression_term) const
            {
                gen->generateTerm(expression_term);
            }
            void operator()(const NodeExpressionBinary *expression_binary) const
            {
                gen->generateBinaryExpression(expression_binary);
            }
        };

        ExpressionVisitor visitor(this);
        std::visit(visitor, expression->var);
    };

    void Assembler::generateScope(const NodeScope *scope)
    {
        begin_scope();
        for (const NodeStatement *statement : scope->statements)
        {
            generateStatement(statement);
        }
        end_scope();
    };

    void Assembler::generateIfPred(const NodeIfPred *pred, const std::string &end_label)
    {
        struct PredVisitor
        {
            Assembler *gen;
            const std::string &end_label;
            PredVisitor(Assembler *gen, const std::string &end_label) : gen(gen), end_label(end_label) {}

            void operator()(const NodeIfPredElif *pred_elif)
            {
                gen->generateExpression(pred_elif->expr);
                gen->pop("rax");
                std::string label = gen->create_label();
                gen->m_output << "\ttest rax, rax\n";
                gen->m_output << "\tjz " << label << "\n";
                gen->generateScope(pred_elif->scope);
                gen->m_output << "\tjmp " << end_label << "\n";
                if (pred_elif->pred.has_value())
                {
                    gen->m_output << label << ":\n";
                    gen->generateIfPred(pred_elif->pred.value(), end_label);
                }
            }

            void operator()(const NodeIfPredElse *pred_else)
            {
                gen->generateScope(pred_else->scope);
            }
        };

        PredVisitor visitor(this, end_label);
        std::visit(visitor, pred->var);
    };

    void Assembler::generateStatement(const NodeStatement *statement)
    {
        struct StatementVisitor
        {
            Assembler *gen;
            StatementVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeStatementExit *statement_exit)
            {
                gen->m_output << "; exit\n";
                gen->generateExpression(statement_exit->expression); // ASM: Expression
                gen->pop("rcx");                                     // ASM: Pop into Exit Code Register (rcx)
                gen->alignStackAndCall("ExitProcess");               // ASM: Call ExitProcess
                gen->m_output << "; /exit\n";
            }
            void operator()(const NodeStatementLet *statement_let)
            {
                gen->m_output << "; let\n";
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == statement_let->ident.value.value(); });
                if (it != gen->m_vars.cend())
                {
                    LOG_ERROR("Identifier '{}' exists already", statement_let->ident.value.value());
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.push_back(Var{statement_let->ident.value.value(), gen->m_stack_size});
                gen->generateExpression(statement_let->expression);
                gen->m_output << "; /let\n";
            }
            void operator()(const NodeStatementAssign *assign)
            {
                gen->m_output << "; assign\n";
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == assign->ident.value.value(); });
                if (it == gen->m_vars.cend())
                {
                    LOG_ERROR("Undeclared identifier {}", assign->ident.value.value());
                    exit(EXIT_FAILURE);
                }
                gen->generateExpression(assign->expression);
                gen->pop("rax");
                size_t offset = (gen->m_stack_size - (*it).stack_loc - 1) * 8;
                gen->m_output << "\tmov [rsp+" << std::to_string(offset) << "], rax\n";
                gen->m_output << "; /assign\n";
            }
            void operator()(const NodeScope *scope)
            {
                gen->generateScope(scope);
            }
            void operator()(const NodeStatementIf *statement_if)
            {
                gen->m_output << "; if\n";
                gen->generateExpression(statement_if->expr);
                gen->pop("rax");
                std::string label = gen->create_label();
                gen->m_output << "\ttest rax, rax\n";
                gen->m_output << "\tjz " << label << "\n";
                gen->generateScope(statement_if->scope);
                if (statement_if->pred.has_value())
                {
                    std::string end_label = gen->create_label();
                    gen->m_output << "\tjmp " << end_label << "\n";
                    gen->m_output << label << ":\n";
                    gen->generateIfPred(statement_if->pred.value(), end_label);
                    gen->m_output << end_label << ":\n";
                }
                else
                {

                    gen->m_output << label << ":\n";
                }
                gen->m_output << "; /if\n";
            }
        };

        StatementVisitor visitor(this);
        std::visit(visitor, statement->var);
    }

    void
    Assembler::alignStackAndCall(const std::string &function)
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
        m_output << "\tpush " << reg << "\n";
        m_stack_size++;
    }

    void Assembler::pop(const std::string &reg)
    {
        m_output << "\tpop " << reg << "\n";
        m_stack_size--;
    }

    std::string Assembler::create_label()
    {
        std::string name = "label" + std::to_string(m_label_count);
        m_label_count++;
        return name;
    }

    void Assembler::begin_scope()
    {
        m_scopes.push_back(m_vars.size());
        m_output << "\t; Begin Scope " << m_scopes.size() << "\n";
    }

    void Assembler::end_scope()
    {
        size_t pop_count = m_vars.size() - m_scopes.back();
        if (pop_count > 0)
        {
            m_output << "\tadd rsp, " << pop_count * 8 << " ; Clean up " << pop_count << " " + std::string(pop_count == 1 ? "variable" : "variables") + "\n";
            m_stack_size -= pop_count;
        }
        m_output << "\t; End Scope " << m_scopes.size() << "\n";
        for (int i = 0; i < pop_count; i++)
        {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }
} // namespace Delta