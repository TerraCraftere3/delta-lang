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
                // Integer literals default to INT32, but we could infer based on value
                DataType literalType = DataType::INT32;
                std::string reg = gen->getAppropriateRegister(literalType);

                gen->m_output << "\tmov " << reg << ", " << term_int_lit->int_literal.value.value() << "\n";
                gen->pushTyped("rax", literalType); // Always push full register for stack alignment
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
                DataType varType = (*it).type;

                // Load with appropriate size
                std::string reg = gen->getAppropriateRegister(varType);
                gen->m_output << "\tmov " << reg << ", ";

                switch (varType)
                {
                case DataType::INT8:
                    gen->m_output << "BYTE [rsp+" << offset << "]\n";
                    break;
                case DataType::INT16:
                    gen->m_output << "WORD [rsp+" << offset << "]\n";
                    break;
                case DataType::INT32:
                    gen->m_output << "DWORD [rsp+" << offset << "]\n";
                    break;
                case DataType::INT64:
                    gen->m_output << "QWORD [rsp+" << offset << "]\n";
                    break;
                default:
                    gen->m_output << "QWORD [rsp+" << offset << "]\n";
                    break;
                }

                gen->pushTyped("rax", varType);
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
                DataType rightType = gen->inferExpressionType(sub->right);
                DataType leftType = gen->inferExpressionType(sub->left);

                gen->generateExpression(sub->right);
                gen->generateExpression(sub->left);

                gen->generateTypedBinaryOp("sub", leftType, rightType);
            }
            void operator()(const NodeExpressionBinaryAddition *add) const
            {
                DataType rightType = gen->inferExpressionType(add->right);
                DataType leftType = gen->inferExpressionType(add->left);

                gen->generateExpression(add->right);
                gen->generateExpression(add->left);

                gen->generateTypedBinaryOp("add", leftType, rightType);
            }
            void operator()(const NodeExpressionBinaryMultiplication *mul) const
            {
                DataType rightType = gen->inferExpressionType(mul->right);
                DataType leftType = gen->inferExpressionType(mul->left);

                gen->generateExpression(mul->right);
                gen->generateExpression(mul->left);

                gen->generateTypedBinaryOp("imul", leftType, rightType); // Use imul for signed multiplication
            }
            void operator()(const NodeExpressionBinaryDivision *div) const
            {
                DataType rightType = gen->inferExpressionType(div->right);
                DataType leftType = gen->inferExpressionType(div->left);

                gen->generateExpression(div->right);
                gen->generateExpression(div->left);

                // Division requires special handling
                gen->pop("rax");
                gen->pop("rbx");

                // Sign extend for signed division
                switch (leftType)
                {
                case DataType::INT8:
                    gen->m_output << "\tcbw\n"; // AL -> AX
                    gen->m_output << "\tidiv bl\n";
                    break;
                case DataType::INT16:
                    gen->m_output << "\tcwd\n"; // AX -> DX:AX
                    gen->m_output << "\tidiv bx\n";
                    break;
                case DataType::INT32:
                    gen->m_output << "\tcdq\n"; // EAX -> EDX:EAX
                    gen->m_output << "\tidiv ebx\n";
                    break;
                case DataType::INT64:
                    gen->m_output << "\tcqo\n"; // RAX -> RDX:RAX
                    gen->m_output << "\tidiv rbx\n";
                    break;
                default:
                    gen->m_output << "\tcqo\n";
                    gen->m_output << "\tidiv rbx\n";
                    break;
                }

                // Result type is the larger of the two operand types
                DataType resultType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;
                gen->pushTyped("rax", resultType);
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

            void operator()(const NodeExpressionTerm *expression_term) const
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
                else
                {
                    gen->m_output << label << ":\n";
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
                gen->generateExpression(statement_exit->expression);
                gen->pop("rcx"); // Exit code must be in RCX for Windows
                gen->alignStackAndCall("ExitProcess");
                gen->m_output << "; /exit\n";
            }
            void operator()(const NodeStatementLet *statement_let)
            {
                gen->m_output << "; let " << typeToString(statement_let->type) << " " << statement_let->ident.value.value() << "\n";

                // Check if identifier already exists
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == statement_let->ident.value.value(); });
                if (it != gen->m_vars.cend())
                {
                    LOG_ERROR("Identifier '{}' exists already", statement_let->ident.value.value());
                    exit(EXIT_FAILURE);
                }

                // Type check the expression
                DataType exprType = gen->inferExpressionType(statement_let->expression);
                gen->validateTypeCompatibility(statement_let->type, exprType, "variable declaration");

                // Add variable before generating expression (for recursion support)
                Var var = Var(statement_let->ident.value.value(), gen->m_stack_size, statement_let->type);
                var.setConstant(statement_let->isConst);
                gen->m_vars.push_back(var);
                gen->generateExpression(statement_let->expression);

                gen->m_output << "; /let " << typeToString(statement_let->type) << "\n";
            }
            void operator()(const NodeStatementAssign *assign)
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == assign->ident.value.value(); });
                if (it == gen->m_vars.cend())
                {
                    LOG_ERROR("Undeclared identifier {}", assign->ident.value.value());
                    exit(EXIT_FAILURE);
                }

                Var var = (*it);
                if (var.isConstant)
                {
                    LOG_ERROR("Variable {} is constant", var.name);
                    exit(EXIT_FAILURE);
                }
                gen->m_output << "; assign " << typeToString(var.type) << " " << var.name << "\n";

                // Type check the assignment
                DataType exprType = gen->inferExpressionType(assign->expression);
                gen->validateTypeCompatibility(var.type, exprType, "assignment");

                gen->generateExpression(assign->expression);
                gen->pop("rax");

                size_t offset = (gen->m_stack_size - var.stack_loc - 1) * 8;

                // Store with appropriate size
                std::string reg = gen->getAppropriateRegister(var.type);
                switch (var.type)
                {
                case DataType::INT8:
                    gen->m_output << "\tmov BYTE [rsp+" << offset << "], " << gen->getAppropriateRegister(var.type, "rax") << "\n";
                    break;
                case DataType::INT16:
                    gen->m_output << "\tmov WORD [rsp+" << offset << "], " << gen->getAppropriateRegister(var.type, "rax") << "\n";
                    break;
                case DataType::INT32:
                    gen->m_output << "\tmov DWORD [rsp+" << offset << "], " << gen->getAppropriateRegister(var.type, "rax") << "\n";
                    break;
                case DataType::INT64:
                    gen->m_output << "\tmov QWORD [rsp+" << offset << "], rax\n";
                    break;
                default:
                    gen->m_output << "\tmov [rsp+" << offset << "], rax\n";
                    break;
                }

                gen->m_output << "; /assign " << typeToString(var.type) << "\n";
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

    void Assembler::alignStackAndCall(const std::string &function)
    {
        size_t current_stack_bytes = m_stack_size * 8;
        size_t misalignment = current_stack_bytes % 16;

        if (misalignment != 8)
        {
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
        m_stack_byte_size += 8;
    }

    void Assembler::pop(const std::string &reg)
    {
        m_output << "\tpop " << reg << "\n";
        m_stack_size--;
        m_stack_byte_size -= 8;
    }

    void Assembler::pushTyped(const std::string &reg, DataType type)
    {
        // Always push full 64-bit for stack alignment, but track the type
        push(reg);
    }

    void Assembler::popTyped(const std::string &reg, DataType type)
    {
        // Always pop full 64-bit for stack alignment
        pop(reg);
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
        m_output << "; Begin Scope " << m_scopes.size() << "\n";
    }

    void Assembler::end_scope()
    {
        size_t pop_count = m_vars.size() - m_scopes.back();
        if (pop_count > 0)
        {
            m_output << "\tadd rsp, " << pop_count * 8 << " ; Clean up " << pop_count << " " + std::string(pop_count == 1 ? "variable" : "variables") + " (" << pop_count * 8 << " bytes)\n";
            m_stack_size -= pop_count;
            m_stack_byte_size -= pop_count * 8;
        }
        m_output << "; End Scope " << m_scopes.size() << "\n";
        for (int i = 0; i < pop_count; i++)
        {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }

    std::string Assembler::getAppropriateRegister(DataType type, const std::string &base_reg)
    {
        return getRegisterName(type, base_reg);
    }

    void Assembler::generateTypedMove(const std::string &dest, const std::string &src, DataType type)
    {
        std::string dest_reg = getAppropriateRegister(type, dest);
        std::string src_reg = getAppropriateRegister(type, src);
        m_output << "\tmov " << dest_reg << ", " << src_reg << "\n";
    }

    void Assembler::generateTypedBinaryOp(const std::string &op, DataType leftType, DataType rightType)
    {
        // Determine result type (promote to larger type)
        DataType resultType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

        pop("rax"); // left operand
        pop("rbx"); // right operand

        // Get appropriate register names
        std::string left_reg = getAppropriateRegister(resultType, "rax");
        std::string right_reg = getAppropriateRegister(resultType, "rbx");

        if (op == "imul")
        {
            // Multiplication has special syntax
            m_output << "\t" << op << " " << left_reg << ", " << right_reg << "\n";
        }
        else
        {
            m_output << "\t" << op << " " << left_reg << ", " << right_reg << "\n";
        }

        pushTyped("rax", resultType);
    }

    DataType Assembler::inferExpressionType(const NodeExpression *expression)
    {
        // Check cache first
        auto it = m_expression_types.find(expression);
        if (it != m_expression_types.end())
        {
            return it->second;
        }

        struct ExpressionTypeVisitor
        {
            Assembler *gen;
            ExpressionTypeVisitor(Assembler *gen) : gen(gen) {}

            DataType operator()(const NodeExpressionTerm *expression_term) const
            {
                return gen->inferTermType(expression_term);
            }
            DataType operator()(const NodeExpressionBinary *expression_binary) const
            {
                return gen->inferBinaryExpressionType(expression_binary);
            }
        };

        ExpressionTypeVisitor visitor(this);
        DataType type = std::visit(visitor, expression->var);

        // Cache the result
        m_expression_types[expression] = type;
        return type;
    }

    DataType Assembler::inferTermType(const NodeExpressionTerm *term)
    {
        struct TermTypeVisitor
        {
            Assembler *gen;
            TermTypeVisitor(Assembler *gen) : gen(gen) {}

            DataType operator()(const NodeTermIntegerLiteral *term_int_lit) const
            {
                // Could infer based on literal value, but default to INT32
                return DataType::INT32;
            }
            DataType operator()(const NodeTermIdentifier *term_ident) const
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == term_ident->ident.value.value(); });
                if (it == gen->m_vars.cend())
                {
                    LOG_ERROR("Undeclared identifier {}", term_ident->ident.value.value());
                    exit(EXIT_FAILURE);
                }
                return (*it).type;
            }
            DataType operator()(const NodeTermParen *term_paren) const
            {
                return gen->inferExpressionType(term_paren->expr);
            }
        };

        TermTypeVisitor visitor(this);
        return std::visit(visitor, term->var);
    }

    DataType Assembler::inferBinaryExpressionType(const NodeExpressionBinary *bin_expr)
    {
        struct BinaryExpressionTypeVisitor
        {
            Assembler *gen;
            BinaryExpressionTypeVisitor(Assembler *gen) : gen(gen) {}

            DataType operator()(const NodeExpressionBinarySubtraction *sub) const
            {
                DataType leftType = gen->inferExpressionType(sub->left);
                DataType rightType = gen->inferExpressionType(sub->right);
                return (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;
            }
            DataType operator()(const NodeExpressionBinaryAddition *add) const
            {
                DataType leftType = gen->inferExpressionType(add->left);
                DataType rightType = gen->inferExpressionType(add->right);
                return (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;
            }
            DataType operator()(const NodeExpressionBinaryMultiplication *mul) const
            {
                DataType leftType = gen->inferExpressionType(mul->left);
                DataType rightType = gen->inferExpressionType(mul->right);
                return (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;
            }
            DataType operator()(const NodeExpressionBinaryDivision *div) const
            {
                DataType leftType = gen->inferExpressionType(div->left);
                DataType rightType = gen->inferExpressionType(div->right);
                return (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;
            }
        };

        BinaryExpressionTypeVisitor visitor(this);
        return std::visit(visitor, bin_expr->var);
    }

    void Assembler::validateTypeCompatibility(DataType expected, DataType actual, const std::string &context)
    {
        if (!isTypeCompatible(expected, actual))
        {
            LOG_ERROR("Type mismatch in {}: expected {} but got {}",
                      context, typeToString(expected), typeToString(actual));
            exit(EXIT_FAILURE);
        }
    }

} // namespace Delta