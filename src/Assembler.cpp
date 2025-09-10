#include "Assembler.h"

#include "Log.h"
#include <algorithm>

namespace Delta
{
    Assembler::Assembler(NodeProgram root) : m_program(root), m_current_block_id(0), m_temp_counter(0)
    {
        registerBuiltinFunctions();
        registerExternalFunctions();
    }

    std::string Assembler::generate()
    {
        // LLVM IR module header
        m_output << "; ModuleID = 'delta_program'\n";
        m_output << "target datalayout = \"e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128\"\n";
        m_output << "target triple = \"x86_64-pc-windows-msvc\"\n\n";

        // Forward declare all functions
        // declareFunctions();

        // Generate external function declarations
        generateExternDeclarations();

        // Generate all function definitions
        for (const NodeFunctionDeclaration *func : m_program.functions)
        {
            generateFunctionDeclaration(func);
        }

        return m_output.str();
    }

    void Assembler::declareFunctions()
    {
        m_output << "; Function declarations\n";

        for (const NodeFunctionDeclaration *func : m_program.functions)
        {
            std::string return_type = dataTypeToLLVM(func->return_type);
            m_output << "declare " << return_type << " @" << func->function_name.value.value() << "(";

            for (size_t i = 0; i < func->parameters.size(); i++)
            {
                if (i > 0)
                    m_output << ", ";
                m_output << dataTypeToLLVM(func->parameters[i]->type);
            }

            m_output << ")\n";
        }
        m_output << "\n";
    }

    void Assembler::generateFunctionDeclaration(const NodeFunctionDeclaration *func_decl)
    {
        // Register function
        std::vector<DataType> param_types;
        for (const NodeParameter *param : func_decl->parameters)
        {
            param_types.push_back(param->type);
        }

        Function func(func_decl->function_name.value.value(), param_types,
                      func_decl->return_type, func_decl->function_name.value.value());
        m_functions.push_back(func);

        // Generate function definition
        std::string return_type = dataTypeToLLVM(func_decl->return_type);
        m_output << "define " << return_type << " @" << func_decl->function_name.value.value() << "(";

        // Parameters
        for (size_t i = 0; i < func_decl->parameters.size(); i++)
        {
            if (i > 0)
                m_output << ", ";
            const NodeParameter *param = func_decl->parameters[i];
            m_output << dataTypeToLLVM(param->type) << " %" << param->ident.value.value();
        }

        m_output << ") {\n";

        // Setup function context
        begin_function(func_decl->function_name.value.value());
        m_current_function_return_type = func_decl->return_type;
        m_current_block_id = 0;
        m_temp_counter = 0;

        // Entry block
        m_output << "entry:\n";

        // Allocate space for parameters as local variables
        for (const NodeParameter *param : func_decl->parameters)
        {
            std::string alloca_temp = getNextTemp();
            m_output << "  " << alloca_temp << " = alloca " << dataTypeToLLVM(param->type) << ", align " << getTypeAlignment(param->type) << "\n";
            m_output << "  store " << dataTypeToLLVM(param->type) << " %" << param->ident.value.value() << ", "
                     << dataTypeToLLVM(param->type) << "* " << alloca_temp << ", align " << getTypeAlignment(param->type) << "\n";

            Var var(param->ident.value.value(), 0, param->type);
            var.llvm_alloca = alloca_temp;
            m_vars.push_back(var);
        }

        // Generate function body
        generateScope(func_decl->body);

        // Add default return if none exists
        if (func_decl->return_type == DataType::VOID)
        {
            m_output << "  ret void\n";
        }
        else
        {
            // Add unreachable after last instruction if no explicit return
            m_output << "  ret " << dataTypeToLLVM(func_decl->return_type) << " ";
            generateDefaultValue(func_decl->return_type);
            m_output << "\n";
        }

        end_function();
        m_output << "}\n\n";
    }

    std::string Assembler::generateTerm(const NodeExpressionTerm *term)
    {
        struct TermVisitor
        {
            Assembler *gen;
            TermVisitor(Assembler *gen) : gen(gen) {}

            std::string operator()(const NodeTermIntegerLiteral *term_int_lit) const
            {
                return term_int_lit->int_literal.value.value();
            }

            std::string operator()(const NodeTermIdentifier *term_ident) const
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == term_ident->ident.value.value(); });
                if (it == gen->m_vars.cend())
                {
                    LOG_ERROR("Undeclared identifier {}", term_ident->ident.value.value());
                    exit(EXIT_FAILURE);
                }

                std::string load_temp = gen->getNextTemp();
                gen->m_output << "  " << load_temp << " = load " << gen->dataTypeToLLVM((*it).type)
                              << ", " << gen->dataTypeToLLVM((*it).type) << "* " << (*it).llvm_alloca
                              << ", align " << gen->getTypeAlignment((*it).type) << "\n";
                return load_temp;
            }

            std::string operator()(const NodeTermParen *term_paren) const
            {
                return gen->generateExpression(term_paren->expr);
            }

            std::string operator()(const NodeTermFunctionCall *func_call) const
            {
                return gen->generateFunctionCall(func_call);
            }
        };

        TermVisitor visitor(this);
        return std::visit(visitor, term->var);
    }

    std::string Assembler::generateFunctionCall(const NodeTermFunctionCall *func_call)
    {
        std::string func_name = func_call->function_name.value.value();

        // Validate function call
        validateFunctionCall(func_name, func_call->arguments);

        Function *func = findFunction(func_name);
        if (!func)
        {
            LOG_ERROR("Unknown function: {}", func_name);
            exit(EXIT_FAILURE);
        }

        // Generate arguments
        std::vector<std::string> arg_values;
        std::vector<DataType> arg_types;

        for (const NodeExpression *arg : func_call->arguments)
        {
            std::string arg_val = generateExpression(arg);
            DataType arg_type = inferExpressionType(arg);
            arg_values.push_back(arg_val);
            arg_types.push_back(arg_type);
        }

        // Generate call
        if (func->return_type == DataType::VOID)
        {
            m_output << "  call void @" << func_name << "(";
        }
        else
        {
            std::string result_temp = getNextTemp();
            m_output << "  " << result_temp << " = call " << dataTypeToLLVM(func->return_type) << " @" << func_name << "(";

            // Add arguments
            for (size_t i = 0; i < arg_values.size(); i++)
            {
                if (i > 0)
                    m_output << ", ";
                m_output << dataTypeToLLVM(arg_types[i]) << " " << arg_values[i];
            }

            m_output << ")\n";
            return result_temp;
        }

        // Add arguments for void functions
        for (size_t i = 0; i < arg_values.size(); i++)
        {
            if (i > 0)
                m_output << ", ";
            m_output << dataTypeToLLVM(arg_types[i]) << " " << arg_values[i];
        }

        m_output << ")\n";
        return ""; // Void return
    }

    std::string Assembler::generateBinaryExpression(const NodeExpressionBinary *bin_expr)
    {
        struct BinaryExpressionVisitor
        {
            Assembler *gen;
            BinaryExpressionVisitor(Assembler *gen) : gen(gen) {}

            std::string operator()(const NodeExpressionBinaryAddition *add) const
            {
                std::string left = gen->generateExpression(add->left);
                std::string right = gen->generateExpression(add->right);

                DataType leftType = gen->inferExpressionType(add->left);
                DataType rightType = gen->inferExpressionType(add->right);
                DataType resultType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = add " << gen->dataTypeToLLVM(resultType)
                              << " " << left << ", " << right << "\n";
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinarySubtraction *sub) const
            {
                std::string left = gen->generateExpression(sub->left);
                std::string right = gen->generateExpression(sub->right);

                DataType leftType = gen->inferExpressionType(sub->left);
                DataType rightType = gen->inferExpressionType(sub->right);
                DataType resultType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = sub " << gen->dataTypeToLLVM(resultType)
                              << " " << left << ", " << right << "\n";
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinaryMultiplication *mul) const
            {
                std::string left = gen->generateExpression(mul->left);
                std::string right = gen->generateExpression(mul->right);

                DataType leftType = gen->inferExpressionType(mul->left);
                DataType rightType = gen->inferExpressionType(mul->right);
                DataType resultType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = mul " << gen->dataTypeToLLVM(resultType)
                              << " " << left << ", " << right << "\n";
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinaryDivision *div) const
            {
                std::string left = gen->generateExpression(div->left);
                std::string right = gen->generateExpression(div->right);

                DataType leftType = gen->inferExpressionType(div->left);
                DataType rightType = gen->inferExpressionType(div->right);
                DataType resultType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = sdiv " << gen->dataTypeToLLVM(resultType)
                              << " " << left << ", " << right << "\n";
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinaryGreater *gt) const
            {
                std::string left = gen->generateExpression(gt->left);
                std::string right = gen->generateExpression(gt->right);

                DataType leftType = gen->inferExpressionType(gt->left);
                DataType rightType = gen->inferExpressionType(gt->right);
                DataType compareType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = icmp sgt " << gen->dataTypeToLLVM(compareType)
                              << " " << left << ", " << right << "\n";

                // Convert i1 to i32
                std::string final_temp = gen->getNextTemp();
                gen->m_output << "  " << final_temp << " = zext i1 " << result_temp << " to i32\n";
                return final_temp;
            }

            std::string operator()(const NodeExpressionBinaryGreaterEquals *gte) const
            {
                std::string left = gen->generateExpression(gte->left);
                std::string right = gen->generateExpression(gte->right);

                DataType leftType = gen->inferExpressionType(gte->left);
                DataType rightType = gen->inferExpressionType(gte->right);
                DataType compareType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = icmp sge " << gen->dataTypeToLLVM(compareType)
                              << " " << left << ", " << right << "\n";

                std::string final_temp = gen->getNextTemp();
                gen->m_output << "  " << final_temp << " = zext i1 " << result_temp << " to i32\n";
                return final_temp;
            }

            std::string operator()(const NodeExpressionBinaryLess *lt) const
            {
                std::string left = gen->generateExpression(lt->left);
                std::string right = gen->generateExpression(lt->right);

                DataType leftType = gen->inferExpressionType(lt->left);
                DataType rightType = gen->inferExpressionType(lt->right);
                DataType compareType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = icmp slt " << gen->dataTypeToLLVM(compareType)
                              << " " << left << ", " << right << "\n";

                std::string final_temp = gen->getNextTemp();
                gen->m_output << "  " << final_temp << " = zext i1 " << result_temp << " to i32\n";
                return final_temp;
            }

            std::string operator()(const NodeExpressionBinaryLessEquals *lte) const
            {
                std::string left = gen->generateExpression(lte->left);
                std::string right = gen->generateExpression(lte->right);

                DataType leftType = gen->inferExpressionType(lte->left);
                DataType rightType = gen->inferExpressionType(lte->right);
                DataType compareType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = icmp sle " << gen->dataTypeToLLVM(compareType)
                              << " " << left << ", " << right << "\n";

                std::string final_temp = gen->getNextTemp();
                gen->m_output << "  " << final_temp << " = zext i1 " << result_temp << " to i32\n";
                return final_temp;
            }

            std::string operator()(const NodeExpressionBinaryEquals *eq) const
            {
                std::string left = gen->generateExpression(eq->left);
                std::string right = gen->generateExpression(eq->right);

                DataType leftType = gen->inferExpressionType(eq->left);
                DataType rightType = gen->inferExpressionType(eq->right);
                DataType compareType = (getTypeSize(leftType) >= getTypeSize(rightType)) ? leftType : rightType;

                std::string result_temp = gen->getNextTemp();
                gen->m_output << "  " << result_temp << " = icmp eq " << gen->dataTypeToLLVM(compareType)
                              << " " << left << ", " << right << "\n";

                std::string final_temp = gen->getNextTemp();
                gen->m_output << "  " << final_temp << " = zext i1 " << result_temp << " to i32\n";
                return final_temp;
            }
        };

        BinaryExpressionVisitor visitor(this);
        return std::visit(visitor, bin_expr->var);
    }

    std::string Assembler::generateExpression(const NodeExpression *expression)
    {
        struct ExpressionVisitor
        {
            Assembler *gen;
            ExpressionVisitor(Assembler *gen) : gen(gen) {}

            std::string operator()(const NodeExpressionTerm *expression_term) const
            {
                return gen->generateTerm(expression_term);
            }
            std::string operator()(const NodeExpressionBinary *expression_binary) const
            {
                return gen->generateBinaryExpression(expression_binary);
            }
        };

        ExpressionVisitor visitor(this);
        return std::visit(visitor, expression->var);
    }

    void Assembler::generateScope(const NodeScope *scope)
    {
        begin_scope();
        for (const NodeStatement *statement : scope->statements)
        {
            generateStatement(statement);
        }
        end_scope();
    }

    void Assembler::generateIfPred(const NodeIfPred *pred, const std::string &merge_label)
    {
        struct PredVisitor
        {
            Assembler *gen;
            const std::string &merge_label;
            PredVisitor(Assembler *gen, const std::string &merge_label) : gen(gen), merge_label(merge_label) {}

            void operator()(const NodeIfPredElif *pred_elif)
            {
                std::string cond = gen->generateExpression(pred_elif->expr);
                std::string true_label = gen->getNextLabel();
                std::string false_label = gen->getNextLabel();

                // Convert condition to i1 if needed
                std::string bool_cond = gen->getNextTemp();
                gen->m_output << "  " << bool_cond << " = icmp ne i32 " << cond << ", 0\n";
                gen->m_output << "  br i1 " << bool_cond << ", label %" << true_label << ", label %" << false_label << "\n\n";

                // True branch
                gen->m_output << true_label << ":\n";
                gen->generateScope(pred_elif->scope);
                gen->m_output << "  br label %" << merge_label << "\n\n";

                // False branch
                gen->m_output << false_label << ":\n";
                if (pred_elif->pred.has_value())
                {
                    gen->generateIfPred(pred_elif->pred.value(), merge_label);
                }
                else
                {
                    gen->m_output << "  br label %" << merge_label << "\n";
                }
            }

            void operator()(const NodeIfPredElse *pred_else)
            {
                gen->generateScope(pred_else->scope);
                gen->m_output << "  br label %" << merge_label << "\n";
            }
        };

        PredVisitor visitor(this, merge_label);
        std::visit(visitor, pred->var);
    }

    void Assembler::generateStatement(const NodeStatement *statement)
    {
        struct StatementVisitor
        {
            Assembler *gen;
            StatementVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeStatementExit *statement_exit)
            {
                std::string exit_code = gen->generateExpression(statement_exit->expression);
                gen->m_output << "  call void @exit(i32 " << exit_code << ")\n";
                gen->m_output << "  unreachable\n";
            }

            void operator()(const NodeStatementLet *statement_let)
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == statement_let->ident.value.value(); });
                if (it != gen->m_vars.cend())
                {
                    LOG_ERROR("Identifier '{}' exists already", statement_let->ident.value.value());
                    exit(EXIT_FAILURE);
                }

                DataType exprType = gen->inferExpressionType(statement_let->expression);
                gen->validateTypeCompatibility(statement_let->type, exprType, "variable declaration");

                // Allocate space for variable
                std::string alloca_temp = gen->getNextTemp();
                gen->m_output << "  " << alloca_temp << " = alloca " << gen->dataTypeToLLVM(statement_let->type)
                              << ", align " << gen->getTypeAlignment(statement_let->type) << "\n";

                // Generate expression and store
                std::string expr_value = gen->generateExpression(statement_let->expression);
                gen->m_output << "  store " << gen->dataTypeToLLVM(statement_let->type) << " " << expr_value
                              << ", " << gen->dataTypeToLLVM(statement_let->type) << "* " << alloca_temp
                              << ", align " << gen->getTypeAlignment(statement_let->type) << "\n";

                Var var = Var(statement_let->ident.value.value(), 0, statement_let->type);
                var.setConstant(statement_let->isConst);
                var.llvm_alloca = alloca_temp;
                gen->m_vars.push_back(var);
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

                DataType exprType = gen->inferExpressionType(assign->expression);
                gen->validateTypeCompatibility(var.type, exprType, "assignment");

                std::string expr_value = gen->generateExpression(assign->expression);
                gen->m_output << "  store " << gen->dataTypeToLLVM(var.type) << " " << expr_value
                              << ", " << gen->dataTypeToLLVM(var.type) << "* " << var.llvm_alloca
                              << ", align " << gen->getTypeAlignment(var.type) << "\n";
            }

            void operator()(const NodeScope *scope)
            {
                gen->generateScope(scope);
            }

            void operator()(const NodeStatementIf *statement_if)
            {
                std::string cond = gen->generateExpression(statement_if->expr);
                std::string true_label = gen->getNextLabel();
                std::string merge_label = gen->getNextLabel();
                std::string false_label = statement_if->pred.has_value() ? gen->getNextLabel() : merge_label;

                // Convert condition to i1 if needed
                std::string bool_cond = gen->getNextTemp();
                gen->m_output << "  " << bool_cond << " = icmp ne i32 " << cond << ", 0\n";
                gen->m_output << "  br i1 " << bool_cond << ", label %" << true_label << ", label %" << false_label << "\n\n";

                // True branch
                gen->m_output << true_label << ":\n";
                gen->generateScope(statement_if->scope);
                gen->m_output << "  br label %" << merge_label << "\n\n";

                // False branch (elif/else)
                if (statement_if->pred.has_value())
                {
                    gen->m_output << false_label << ":\n";
                    gen->generateIfPred(statement_if->pred.value(), merge_label);
                    gen->m_output << "\n";
                }

                // Merge point
                gen->m_output << merge_label << ":\n";
            }

            void operator()(const NodeStatementReturn *statement_return)
            {
                if (statement_return->expression)
                {
                    if (gen->m_current_function_return_type == DataType::VOID)
                    {
                        LOG_ERROR("Cannot return value from void function");
                        exit(EXIT_FAILURE);
                    }

                    DataType exprType = gen->inferExpressionType(statement_return->expression);
                    gen->validateTypeCompatibility(gen->m_current_function_return_type, exprType, "return statement");

                    std::string return_value = gen->generateExpression(statement_return->expression);
                    gen->m_output << "  ret " << gen->dataTypeToLLVM(gen->m_current_function_return_type)
                                  << " " << return_value << "\n";
                }
                else
                {
                    if (gen->m_current_function_return_type != DataType::VOID)
                    {
                        LOG_ERROR("Must return value from non-void function");
                        exit(EXIT_FAILURE);
                    }
                    gen->m_output << "  ret void\n";
                }
            }
        };

        StatementVisitor visitor(this);
        std::visit(visitor, statement->var);
    }

    // Helper methods

    std::string Assembler::getNextTemp()
    {
        return "%t" + std::to_string(m_temp_counter++);
    }

    std::string Assembler::getNextLabel()
    {
        return "bb" + std::to_string(m_current_block_id++);
    }

    std::string Assembler::dataTypeToLLVM(DataType type)
    {
        switch (type)
        {
        case DataType::INT8:
            return "i8";
        case DataType::INT16:
            return "i16";
        case DataType::INT32:
            return "i32";
        case DataType::INT64:
            return "i64";
        case DataType::VOID:
            return "void";
        default:
            return "i32";
        }
    }

    int Assembler::getTypeAlignment(DataType type)
    {
        switch (type)
        {
        case DataType::INT8:
            return 1;
        case DataType::INT16:
            return 2;
        case DataType::INT32:
            return 4;
        case DataType::INT64:
            return 8;
        default:
            return 4;
        }
    }

    void Assembler::generateDefaultValue(DataType type)
    {
        switch (type)
        {
        case DataType::INT8:
        case DataType::INT16:
        case DataType::INT32:
        case DataType::INT64:
            m_output << "0";
            break;
        case DataType::VOID:
            // Should not happen
            break;
        default:
            m_output << "0";
            break;
        }
    }

    void Assembler::generateTypeConversion(const std::string &value, DataType from, DataType to)
    {
        if (from == to)
        {
            m_output << value;
            return;
        }

        int from_bits = getTypeSize(from) * 8;
        int to_bits = getTypeSize(to) * 8;

        if (from_bits < to_bits)
        {
            // Sign extend
            m_output << "sext " << dataTypeToLLVM(from) << " " << value << " to " << dataTypeToLLVM(to);
        }
        else if (from_bits > to_bits)
        {
            // Truncate
            m_output << "trunc " << dataTypeToLLVM(from) << " " << value << " to " << dataTypeToLLVM(to);
        }
        else
        {
            // Same size, bitcast
            m_output << "bitcast " << dataTypeToLLVM(from) << " " << value << " to " << dataTypeToLLVM(to);
        }
    }

    void Assembler::begin_scope()
    {
        m_scopes.push_back(m_vars.size());
    }

    void Assembler::end_scope()
    {
        size_t vars_to_remove = m_vars.size() - m_scopes.back();
        for (size_t i = 0; i < vars_to_remove; i++)
        {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }

    void Assembler::begin_function(const std::string &func_name)
    {
        m_current_function = func_name;
        m_in_function = true;
    }

    void Assembler::end_function()
    {
        // Clean up all scopes
        while (!m_scopes.empty())
        {
            end_scope();
        }
        m_vars.clear();
        m_current_function = "";
        m_in_function = false;
        m_current_function_return_type = DataType::VOID;
    }

    // Type inference methods (reused from original)
    DataType Assembler::inferExpressionType(const NodeExpression *expression)
    {
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
            DataType operator()(const NodeTermFunctionCall *func_call) const
            {
                Function *func = gen->findFunction(func_call->function_name.value.value());
                if (!func)
                {
                    LOG_ERROR("Unknown function: {}", func_call->function_name.value.value());
                    exit(EXIT_FAILURE);
                }
                return func->return_type;
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
            DataType operator()(const NodeExpressionBinaryGreater *) const
            {
                return DataType::INT32;
            }
            DataType operator()(const NodeExpressionBinaryGreaterEquals *) const
            {
                return DataType::INT32;
            }
            DataType operator()(const NodeExpressionBinaryLess *) const
            {
                return DataType::INT32;
            }
            DataType operator()(const NodeExpressionBinaryLessEquals *) const
            {
                return DataType::INT32;
            }
            DataType operator()(const NodeExpressionBinaryEquals *) const
            {
                return DataType::INT32;
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

    Function *Assembler::findFunction(const std::string &name)
    {
        auto it = std::find_if(m_functions.begin(), m_functions.end(),
                               [&name](const Function &func)
                               { return func.name == name; });
        return (it != m_functions.end()) ? &(*it) : nullptr;
    }

    void Assembler::registerBuiltinFunctions()
    {
        // Register common C library functions
        Function exit_func("exit", {DataType::INT32}, DataType::VOID, "exit", true);
        m_functions.push_back(exit_func);

        Function printf_func("printf", {DataType::INT64}, DataType::INT32, "printf", true);
        m_functions.push_back(printf_func);

        Function malloc_func("malloc", {DataType::INT64}, DataType::INT64, "malloc", true);
        m_functions.push_back(malloc_func);

        Function free_func("free", {DataType::INT64}, DataType::VOID, "free", true);
        m_functions.push_back(free_func);
    }

    void Assembler::registerExternalFunctions()
    {
        // Mark external functions as used
        m_used_external_functions.insert("exit");
        m_used_external_functions.insert("printf");
        m_used_external_functions.insert("malloc");
        m_used_external_functions.insert("free");
    }

    void Assembler::validateFunctionCall(const std::string &func_name, const std::vector<NodeExpression *> &arguments)
    {
        Function *func = findFunction(func_name);
        if (!func)
        {
            LOG_ERROR("Unknown function: {}", func_name);
            exit(EXIT_FAILURE);
        }

        if (arguments.size() != func->parameter_types.size())
        {
            LOG_ERROR("Function {} expects {} arguments but got {}",
                      func_name, func->parameter_types.size(), arguments.size());
            exit(EXIT_FAILURE);
        }

        // Validate argument types
        for (size_t i = 0; i < arguments.size(); i++)
        {
            DataType argType = inferExpressionType(arguments[i]);
            DataType expectedType = func->parameter_types[i];

            if (!isTypeCompatible(expectedType, argType))
            {
                LOG_ERROR("Argument {} to function {} has wrong type: expected {} but got {}",
                          i + 1, func_name, typeToString(expectedType), typeToString(argType));
                exit(EXIT_FAILURE);
            }
        }
    }

    void Assembler::generateExternDeclarations()
    {
        m_output << "; External function declarations\n";

        for (const std::string &func_name : m_used_external_functions)
        {
            Function *func = findFunction(func_name);
            if (func && func->is_external)
            {
                std::string return_type = dataTypeToLLVM(func->return_type);
                m_output << "declare " << return_type << " @" << func_name << "(";

                for (size_t i = 0; i < func->parameter_types.size(); i++)
                {
                    if (i > 0)
                        m_output << ", ";
                    m_output << dataTypeToLLVM(func->parameter_types[i]);
                }

                m_output << ")\n";
            }
        }
        m_output << "\n";
    }

} // namespace Delta