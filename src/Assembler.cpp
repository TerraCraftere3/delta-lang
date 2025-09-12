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

        collectStringLiterals();
        generateStringLiterals();
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
            m_output << "declare " << return_type << " @"
                     << func->function_name.value.value() << "(";

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

    std::string float32ToLLVM(const std::string &input)
    {
        float value = std::stof(input);

        if (std::isnan(value))
        {
            return "0x7FF8000000000000";
        }
        if (std::isinf(value))
        {
            return value > 0 ? "0x7FF0000000000000" : "0xFFF0000000000000";
        }
        double double_value = static_cast<double>(value);

        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase;

        uint64_t bits;
        std::memcpy(&bits, &double_value, sizeof(double));
        oss << std::setw(16) << std::setfill('0') << bits;

        return oss.str();
    }

    std::string float64ToLLVM(const std::string &input)
    {
        double value = std::stod(input);

        if (std::isnan(value))
        {
            return "0x7FF8000000000000";
        }
        if (std::isinf(value))
        {
            return value > 0 ? "0x7FF0000000000000" : "0xFFF0000000000000";
        }

        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase;

        uint64_t bits;
        std::memcpy(&bits, &value, sizeof(double));
        oss << std::setw(16) << std::setfill('0') << bits;

        return oss.str();
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

            std::string operator()(const NodeTermFloatLiteral *term_float_lit) const
            {
                return float32ToLLVM(term_float_lit->float_literal.value.value());
            }

            std::string operator()(const NodeTermDoubleLiteral *term_double_lit) const
            {
                return float64ToLLVM(term_double_lit->double_literal.value.value());
            }

            std::string operator()(const NodeTermStringLiteral *term_str_lit) const
            {
                std::string str_value = term_str_lit->string_literal.value.value();

                // Find or add string to global list
                auto it = std::find(gen->m_string_literals.begin(),
                                    gen->m_string_literals.end(), str_value);
                size_t index;

                if (it == gen->m_string_literals.end())
                {
                    index = gen->m_string_literals.size();
                    gen->m_string_literals.push_back(str_value);
                }
                else
                {
                    index = std::distance(gen->m_string_literals.begin(), it);
                }

                // Generate getelementptr to get i8* pointer to string
                std::string result_temp = gen->getNextTemp();
                size_t length = str_value.length() + 1;

                gen->m_output << "  " << result_temp << " = getelementptr inbounds ["
                              << length << " x i8], [" << length << " x i8]* @str."
                              << index << ", i64 0, i64 0 ; String literal\n";

                return result_temp;
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
                              << ", align " << getTypeAlignment((*it).type);
                gen->m_output << " ; Use Variable " << term_ident->ident.value.value() << "\n";
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

            std::string operator()(const NodeTermCast *term_cast) const
            {
                std::string value = gen->generateExpression(term_cast->expr);
                DataType from_type = gen->inferExpressionType(term_cast->expr);
                DataType to_type = term_cast->target_type;

                return gen->generateTypeConversion(value, from_type, to_type);
            }

            std::string operator()(const NodeTermAddressOf *term_addr) const
            {
                // &variable - get address of variable
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(),
                                       [&](const Var &var)
                                       {
                                           return var.name == term_addr->ident.value.value();
                                       });
                if (it == gen->m_vars.cend())
                {
                    LOG_ERROR("Undeclared identifier {}", term_addr->ident.value.value());
                    exit(EXIT_FAILURE);
                }

                return (*it).llvm_alloca;
            }

            std::string operator()(const NodeTermDereference *term_deref) const
            {
                std::string ptr_value = gen->generateExpression(term_deref->expr);
                DataType ptr_type = gen->inferExpressionType(term_deref->expr);

                if (!isPointerType(ptr_type))
                {
                    LOG_ERROR("Cannot dereference non-pointer type");
                    exit(EXIT_FAILURE);
                }

                return ptr_value;
            }

            std::string operator()(const NodeTermArrayAccess *array_access) const
            {
                std::string array_ptr = gen->generateExpression(array_access->array_expr);
                std::string index = gen->generateExpression(array_access->index_expr);

                DataType array_type = gen->inferExpressionType(array_access->array_expr);
                DataType index_type = gen->inferExpressionType(array_access->index_expr);

                if (index_type != DataType::INT64)
                {
                    index = gen->generateTypeConversion(index, index_type, DataType::INT64);
                }

                DataType element_type;
                if (isPointerType(array_type))
                {
                    element_type = getPointeeType(array_type);
                }
                else
                {
                    LOG_ERROR("Cannot index non-pointer type");
                    exit(EXIT_FAILURE);
                }

                std::string gep_temp = gen->getNextTemp();
                gen->m_output << "  " << gep_temp << " = getelementptr "
                              << gen->dataTypeToLLVM(element_type) << ", "
                              << gen->dataTypeToLLVM(array_type) << " " << array_ptr
                              << ", i64 " << index << " ; Array index\n";

                std::string load_temp = gen->getNextTemp();
                gen->m_output << "  " << load_temp << " = load "
                              << gen->dataTypeToLLVM(element_type) << ", "
                              << gen->dataTypeToLLVM(element_type) << "* " << gep_temp
                              << ", align " << getTypeAlignment(element_type)
                              << " ; Load array element\n";

                return load_temp;
            }
        };

        TermVisitor visitor(this);
        return std::visit(visitor, term->var);
    }

    std::string Assembler::generateFunctionCall(const NodeTermFunctionCall *func_call)
    {
        std::string func_name = func_call->function_name.value.value();

        validateFunctionCall(func_name, func_call->arguments);

        Function *func = findFunction(func_name);
        if (!func)
        {
            LOG_ERROR("Unknown function: {}", func_name);
            exit(EXIT_FAILURE);
        }

        std::vector<std::string> arg_values;
        std::vector<DataType> arg_types;

        for (size_t i = 0; i < func_call->arguments.size(); i++)
        {
            const NodeExpression *arg = func_call->arguments[i];
            std::string arg_val = generateExpression(arg);
            DataType arg_type = inferExpressionType(arg);

            if (i < func->parameter_types.size())
            {
                DataType expected_type = func->parameter_types[i];
                if (arg_type != expected_type)
                {
                    arg_val = generateTypeConversion(arg_val, arg_type, expected_type);
                    arg_type = expected_type;
                }
            }
            else if (func->is_variadic)
            {
                arg_val = applyDefaultPromotions(arg_val, arg_type);
                arg_type = getPromotedType(arg_type);
            }

            arg_values.push_back(arg_val);
            arg_types.push_back(arg_type);
        }

        if (func->return_type == DataType::VOID)
        {
            m_output << "  call void @" << func_name << "(";

            for (size_t i = 0; i < arg_values.size(); i++)
            {
                if (i > 0)
                    m_output << ", ";
                m_output << dataTypeToLLVM(arg_types[i]) << " " << arg_values[i];
            }

            m_output << ") ; Call " << func_call->function_name.value.value() << "()\n";
            return "";
        }
        else
        {
            std::string result_temp = getNextTemp();
            m_output << "  " << result_temp << " = call " << dataTypeToLLVM(func->return_type)
                     << " @" << func_name << "(";

            for (size_t i = 0; i < arg_values.size(); i++)
            {
                if (i > 0)
                    m_output << ", ";
                m_output << dataTypeToLLVM(arg_types[i]) << " " << arg_values[i];
            }

            m_output << ") ; Call " << func_call->function_name.value.value() << "()\n";
            return result_temp;
        }
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
                DataType resultType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != resultType)
                {
                    left = gen->generateTypeConversion(left, leftType, resultType);
                }
                if (rightType != resultType)
                {
                    right = gen->generateTypeConversion(right, rightType, resultType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(resultType))
                {
                    gen->m_output << "  " << result_temp << " = fadd " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Float Add\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = add " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Add\n";
                }
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinarySubtraction *sub) const
            {
                std::string left = gen->generateExpression(sub->left);
                std::string right = gen->generateExpression(sub->right);

                DataType leftType = gen->inferExpressionType(sub->left);
                DataType rightType = gen->inferExpressionType(sub->right);
                DataType resultType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != resultType)
                {
                    left = gen->generateTypeConversion(left, leftType, resultType);
                }
                if (rightType != resultType)
                {
                    right = gen->generateTypeConversion(right, rightType, resultType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(resultType))
                {
                    gen->m_output << "  " << result_temp << " = fsub " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Float Subtract\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = sub " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Subtract\n";
                }
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinaryMultiplication *mul) const
            {
                std::string left = gen->generateExpression(mul->left);
                std::string right = gen->generateExpression(mul->right);

                DataType leftType = gen->inferExpressionType(mul->left);
                DataType rightType = gen->inferExpressionType(mul->right);
                DataType resultType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != resultType)
                {
                    left = gen->generateTypeConversion(left, leftType, resultType);
                }
                if (rightType != resultType)
                {
                    right = gen->generateTypeConversion(right, rightType, resultType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(resultType))
                {
                    gen->m_output << "  " << result_temp << " = fmul " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Float Multiply\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = mul " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Multiply\n";
                }
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinaryDivision *div) const
            {
                std::string left = gen->generateExpression(div->left);
                std::string right = gen->generateExpression(div->right);

                DataType leftType = gen->inferExpressionType(div->left);
                DataType rightType = gen->inferExpressionType(div->right);
                DataType resultType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != resultType)
                {
                    left = gen->generateTypeConversion(left, leftType, resultType);
                }
                if (rightType != resultType)
                {
                    right = gen->generateTypeConversion(right, rightType, resultType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(resultType))
                {
                    gen->m_output << "  " << result_temp << " = fdiv " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Float Divide\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = sdiv " << gen->dataTypeToLLVM(resultType)
                                  << " " << left << ", " << right << "; Divide\n";
                }
                return result_temp;
            }

            std::string operator()(const NodeExpressionBinaryGreater *gt) const
            {
                std::string left = gen->generateExpression(gt->left);
                std::string right = gen->generateExpression(gt->right);

                DataType leftType = gen->inferExpressionType(gt->left);
                DataType rightType = gen->inferExpressionType(gt->right);
                DataType compareType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != compareType)
                {
                    left = gen->generateTypeConversion(left, leftType, compareType);
                }
                if (rightType != compareType)
                {
                    right = gen->generateTypeConversion(right, rightType, compareType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(compareType))
                {
                    gen->m_output << "  " << result_temp << " = fcmp ogt " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Float Greater Than\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = icmp sgt " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Greater Than\n";
                }

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
                DataType compareType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != compareType)
                {
                    left = gen->generateTypeConversion(left, leftType, compareType);
                }
                if (rightType != compareType)
                {
                    right = gen->generateTypeConversion(right, rightType, compareType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(compareType))
                {
                    gen->m_output << "  " << result_temp << " = fcmp oge " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Float Greater or Equals\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = icmp sge " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Greater or Equals\n";
                }

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
                DataType compareType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != compareType)
                {
                    left = gen->generateTypeConversion(left, leftType, compareType);
                }
                if (rightType != compareType)
                {
                    right = gen->generateTypeConversion(right, rightType, compareType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(compareType))
                {
                    gen->m_output << "  " << result_temp << " = fcmp olt " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Float Less Than\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = icmp slt " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Less Than\n";
                }

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
                DataType compareType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != compareType)
                {
                    left = gen->generateTypeConversion(left, leftType, compareType);
                }
                if (rightType != compareType)
                {
                    right = gen->generateTypeConversion(right, rightType, compareType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(compareType))
                {
                    gen->m_output << "  " << result_temp << " = fcmp ole " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Float Less or Equals\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = icmp sle " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Less or Equals\n";
                }

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
                DataType compareType = gen->getCommonType(leftType, rightType);

                // Convert operands to common type
                if (leftType != compareType)
                {
                    left = gen->generateTypeConversion(left, leftType, compareType);
                }
                if (rightType != compareType)
                {
                    right = gen->generateTypeConversion(right, rightType, compareType);
                }

                std::string result_temp = gen->getNextTemp();
                if (isFloatType(compareType))
                {
                    gen->m_output << "  " << result_temp << " = fcmp oeq " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Float Equals\n";
                }
                else
                {
                    gen->m_output << "  " << result_temp << " = icmp eq " << gen->dataTypeToLLVM(compareType)
                                  << " " << left << ", " << right << "; Equals\n";
                }

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
                DataType condType = gen->inferExpressionType(pred_elif->expr);
                std::string true_label = gen->getNextLabel();
                std::string false_label = gen->getNextLabel();

                // Convert condition to i1
                std::string bool_cond = gen->convertToBoolean(cond, condType);
                gen->m_output << "  br i1 " << bool_cond << ", label %" << true_label << ", label %" << false_label << "; Elif / Else Jump\n\n";

                // True branch
                gen->m_output << true_label << ":\n";
                gen->generateScope(pred_elif->scope);
                gen->m_output << "  br label %" << merge_label << "; Break\n\n";

                // False branch
                gen->m_output << false_label << ":\n";
                if (pred_elif->pred.has_value())
                {
                    gen->generateIfPred(pred_elif->pred.value(), merge_label);
                }
                else
                {
                    gen->m_output << "  br label %" << merge_label << "; Break\n";
                }
            }

            void operator()(const NodeIfPredElse *pred_else)
            {
                gen->generateScope(pred_else->scope);
                gen->m_output << "  br label %" << merge_label << "; Break\n";
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

            void operator()(const NodeExpression *expression)
            {
                gen->generateExpression(expression);
            }

            void operator()(const NodeStatementExit *statement_exit)
            {
                std::string exit_code = gen->generateExpression(statement_exit->expression);
                DataType exprType = gen->inferExpressionType(statement_exit->expression);

                // Convert to int32 for exit code
                if (exprType != DataType::INT32)
                {
                    exit_code = gen->generateTypeConversion(exit_code, exprType, DataType::INT32);
                }

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

                // Allocate space for variable
                std::string alloca_temp = gen->getNextTemp();
                gen->m_output << "  " << alloca_temp << " = alloca " << gen->dataTypeToLLVM(statement_let->type)
                              << ", align " << getTypeAlignment(statement_let->type) << "; Allocate variable \"" << statement_let->ident.value.value() << "\"\n";

                // Generate Expression
                std::string expr_value = gen->generateExpression(statement_let->expression);
                gen->m_output << "  store " << gen->dataTypeToLLVM(statement_let->type) << " " << expr_value
                              << ", " << gen->dataTypeToLLVM(statement_let->type) << "* " << alloca_temp
                              << ", align " << getTypeAlignment(statement_let->type) << "; Set variable \"" << statement_let->ident.value.value() << "\"\n";

                Var var = Var(statement_let->ident.value.value(), 0, statement_let->type);
                var.setConstant(statement_let->isConst);
                var.llvm_alloca = alloca_temp;
                var.type = statement_let->type;
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

                // Generate expression and convert to variable type if needed
                std::string expr_value = gen->generateExpression(assign->expression);
                DataType exprType = gen->inferExpressionType(assign->expression);

                // Convert to variable type if needed
                if (exprType != var.type)
                {
                    expr_value = gen->generateTypeConversion(expr_value, exprType, var.type);
                }

                gen->m_output << "  store " << gen->dataTypeToLLVM(var.type) << " " << expr_value
                              << ", " << gen->dataTypeToLLVM(var.type) << "* " << var.llvm_alloca
                              << ", align " << getTypeAlignment(var.type) << "; Set variable \"" << assign->ident.value.value() << "\"\n";
            }

            void operator()(const NodeScope *scope)
            {
                gen->generateScope(scope);
            }

            void operator()(const NodeStatementIf *statement_if)
            {
                std::string cond = gen->generateExpression(statement_if->expr);
                DataType condType = gen->inferExpressionType(statement_if->expr);
                std::string true_label = gen->getNextLabel();
                std::string merge_label = gen->getNextLabel();
                std::string false_label = statement_if->pred.has_value() ? gen->getNextLabel() : merge_label;

                // Convert condition to i1
                std::string bool_cond = gen->convertToBoolean(cond, condType);
                gen->m_output << "  br i1 " << bool_cond << ", label %" << true_label << ", label %" << false_label << "; If / Else Jump\n\n";

                // True branch
                gen->m_output << true_label << ":\n";
                gen->generateScope(statement_if->scope);
                gen->m_output << "  br label %" << merge_label << "; Break\n\n";

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

                    std::string return_value = gen->generateExpression(statement_return->expression);
                    DataType exprType = gen->inferExpressionType(statement_return->expression);

                    // Convert to function return type if needed
                    if (exprType != gen->m_current_function_return_type)
                    {
                        return_value = gen->generateTypeConversion(return_value, exprType, gen->m_current_function_return_type);
                    }

                    gen->m_output << "  ret " << gen->dataTypeToLLVM(gen->m_current_function_return_type)
                                  << " " << return_value << " ; Return\n";
                }
                else
                {
                    if (gen->m_current_function_return_type != DataType::VOID)
                    {
                        LOG_ERROR("Must return value from non-void function");
                        exit(EXIT_FAILURE);
                    }
                    gen->m_output << "  ret void ; Return\n";
                }
            }
            void operator()(const NodeStatementPointerAssign *ptr_assign)
            {
                std::string ptr_value = gen->generateExpression(ptr_assign->ptr_expr);
                DataType pointee_type = gen->inferExpressionType(ptr_assign->ptr_expr);

                std::string value = gen->generateExpression(ptr_assign->value_expr);
                DataType value_type = gen->inferExpressionType(ptr_assign->value_expr);

                if (value_type != pointee_type)
                {
                    value = gen->generateTypeConversion(value, value_type, pointee_type);
                }

                gen->m_output << "  store " << gen->dataTypeToLLVM(pointee_type) << " " << value
                              << ", " << gen->dataTypeToLLVM(pointee_type) << "* " << ptr_value
                              << ", align " << getTypeAlignment(pointee_type)
                              << " ; Store through pointer\n";
            }
            void operator()(const NodeStatementArrayAssign *array_assign)
            {
                std::string array_ptr = gen->generateExpression(array_assign->array_expr);
                std::string index = gen->generateExpression(array_assign->index_expr);
                std::string value = gen->generateExpression(array_assign->value_expr);

                DataType array_type = gen->inferExpressionType(array_assign->array_expr);
                DataType index_type = gen->inferExpressionType(array_assign->index_expr);
                DataType value_type = gen->inferExpressionType(array_assign->value_expr);

                if (index_type != DataType::INT64)
                {
                    index = gen->generateTypeConversion(index, index_type, DataType::INT64);
                }

                DataType element_type;
                if (isPointerType(array_type))
                {
                    element_type = getPointeeType(array_type);
                }
                else
                {
                    LOG_ERROR("Cannot index non-pointer type");
                    exit(EXIT_FAILURE);
                }

                if (value_type != element_type)
                {
                    value = gen->generateTypeConversion(value, value_type, element_type);
                }

                std::string gep_temp = gen->getNextTemp();
                gen->m_output << "  " << gep_temp << " = getelementptr "
                              << gen->dataTypeToLLVM(element_type) << ", "
                              << gen->dataTypeToLLVM(array_type) << " " << array_ptr
                              << ", i64 " << index << " ; Array index\n";

                gen->m_output << "  store " << gen->dataTypeToLLVM(element_type)
                              << " " << value << ", " << gen->dataTypeToLLVM(element_type)
                              << "* " << gep_temp << ", align " << getTypeAlignment(element_type)
                              << " ; Store array element\n";
            }
        };

        StatementVisitor visitor(this);
        std::visit(visitor, statement->var);
    }

    void Assembler::generateStringLiterals()
    {
        for (size_t i = 0; i < m_string_literals.size(); i++)
        {
            const std::string &str = m_string_literals[i];

            std::string escaped = escapeString(str);
            size_t length = str.length() + 1; // +1 for null terminator

            m_output << "@str." << i << " = private unnamed_addr constant ["
                     << length << " x i8] c\"" << escaped << "\\00\"\n";
        }
        m_output << "\n";
    }

    // Helper methods

    std::string
    Assembler::getNextTemp()
    {
        std::string temp = "%t" + std::to_string(m_temp_counter++);
        return temp;
    }

    std::string Assembler::getNextLabel()
    {
        std::string label = "bb" + std::to_string(m_current_block_id++);
        return label;
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
        case DataType::FLOAT32:
            return "float";
        case DataType::FLOAT64:
            return "double";
        case DataType::VOID:
            return "void";
        // Pointer types
        case DataType::INT8_PTR:
            return "i8*";
        case DataType::INT16_PTR:
            return "i16*";
        case DataType::INT32_PTR:
            return "i32*";
        case DataType::INT64_PTR:
            return "i64*";
        case DataType::FLOAT32_PTR:
            return "float*";
        case DataType::FLOAT64_PTR:
            return "double*";
        case DataType::VOID_PTR:
            return "i8*"; // void* == i8*
        default:
            return "i32";
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
        case DataType::FLOAT32:
            m_output << "0.0";
            break;
        case DataType::FLOAT64:
            m_output << "0.0";
            break;
        case DataType::VOID:
            // Should not happen
            break;
        default:
            m_output << "0";
            break;
        }
    }

    std::string Assembler::generateTypeConversion(const std::string &value, DataType from, DataType to)
    {
        if (from == to)
        {
            return value;
        }

        LOG_TRACE("Converting from {} to {}. Value: {}", typeToString(from), typeToString(to), value);

        std::string result_temp = getNextTemp();

        // Float to float conversions
        if (isFloatType(from) && isFloatType(to))
        {
            int from_bits = getTypeSize(from) * 8;
            int to_bits = getTypeSize(to) * 8;

            if (from_bits < to_bits)
            {
                // Extend float precision (float to double)
                m_output << "  " << result_temp << " = fpext " << dataTypeToLLVM(from)
                         << " " << value << " to " << dataTypeToLLVM(to) << " ; Float Extend\n";
            }
            else if (from_bits > to_bits)
            {
                // Truncate float precision
                m_output << "  " << result_temp << " = fptrunc " << dataTypeToLLVM(from)
                         << " " << value << " to " << dataTypeToLLVM(to) << " ; Float Truncate\n";
            }

            LOG_TRACE("Generated float conversion: {}", result_temp);
            return result_temp;
        }

        // Pointer to pointer conversions (bitcast)
        if (isPointerType(from) && isPointerType(to))
        {
            m_output << "  " << result_temp << " = bitcast " << dataTypeToLLVM(from)
                     << " " << value << " to " << dataTypeToLLVM(to) << " ; Pointer cast\n";
            return result_temp;
        }

        // Integer to pointer conversion (inttoptr)
        if (!isPointerType(from) && isPointerType(to))
        {
            // First convert to i64 if not already
            std::string int_value = value;
            if (from != DataType::INT64)
            {
                std::string temp = getNextTemp();
                if (isFloatType(from))
                {
                    m_output << "  " << temp << " = fptosi " << dataTypeToLLVM(from)
                             << " " << value << " to i64 ; Float to Int64\n";
                }
                else
                {
                    m_output << "  " << temp << " = sext " << dataTypeToLLVM(from)
                             << " " << value << " to i64 ; Int to Int64\n";
                }
                int_value = temp;
            }

            m_output << "  " << result_temp << " = inttoptr i64 " << int_value
                     << " to " << dataTypeToLLVM(to) << " ; Int to Pointer\n";
            return result_temp;
        }

        // Pointer to integer conversion (ptrtoint)
        if (isPointerType(from) && !isPointerType(to))
        {
            std::string ptr_as_int = getNextTemp();
            m_output << "  " << ptr_as_int << " = ptrtoint " << dataTypeToLLVM(from)
                     << " " << value << " to i64 ; Pointer to Int64\n";

            // Then convert i64 to target type if needed
            if (to == DataType::INT64)
            {
                return ptr_as_int;
            }
            else
            {
                return generateTypeConversion(ptr_as_int, DataType::INT64, to);
            }
        }

        if (isFloatType(from) && !isFloatType(to))
        {
            m_output << "  " << result_temp << " = fptosi " << dataTypeToLLVM(from)
                     << " " << value << " to " << dataTypeToLLVM(to) << " ; Float to Int\n";
            return result_temp;
        }

        // Integer to float conversions
        if (!isFloatType(from) && isFloatType(to))
        {
            m_output << "  " << result_temp << " = sitofp " << dataTypeToLLVM(from)
                     << " " << value << " to " << dataTypeToLLVM(to) << " ; Int to Float\n";
            return result_temp;
        }

        // Float to float conversions
        if (isFloatType(from) && isFloatType(to))
        {
            int from_bits = getTypeSize(from) * 8;
            int to_bits = getTypeSize(to) * 8;

            if (from_bits < to_bits)
            {
                // Extend float precision
                m_output << "  " << result_temp << " = fpext " << dataTypeToLLVM(from)
                         << " " << value << " to " << dataTypeToLLVM(to) << " ; Float Extend\n";
            }
            else if (from_bits > to_bits)
            {
                // Truncate float precision
                m_output << "  " << result_temp << " = fptrunc " << dataTypeToLLVM(from)
                         << " " << value << " to " << dataTypeToLLVM(to) << " ; Float Truncate\n";
            }
            else
            {
                // Same size, should not happen
                return value;
            }
            return result_temp;
        }

        // Integer to integer conversions
        int from_bits = getTypeSize(from) * 8;
        int to_bits = getTypeSize(to) * 8;

        if (from_bits < to_bits)
        {
            // Sign extend
            m_output << "  " << result_temp << " = sext " << dataTypeToLLVM(from)
                     << " " << value << " to " << dataTypeToLLVM(to) << " ; Int Sign Extend\n";
        }
        else if (from_bits > to_bits)
        {
            // Truncate
            m_output << "  " << result_temp << " = trunc " << dataTypeToLLVM(from)
                     << " " << value << " to " << dataTypeToLLVM(to) << " ; Int Truncate\n";
        }
        else
        {
            // Same size, bitcast (shouldn't happen for integers of same size but different signedness)
            return value;
        }

        return result_temp;
    }

    std::string Assembler::convertToBoolean(const std::string &value, DataType type)
    {
        std::string bool_temp = getNextTemp();

        if (isFloatType(type))
        {
            // Compare float with 0.0
            m_output << "  " << bool_temp << " = fcmp one " << dataTypeToLLVM(type)
                     << " " << value << ", 0.0 ; Float to Boolean\n";
        }
        else
        {
            // Compare integer with 0
            m_output << "  " << bool_temp << " = icmp ne " << dataTypeToLLVM(type)
                     << " " << value << ", 0 ; Int to Boolean\n";
        }

        return bool_temp;
    }

    DataType Assembler::getCommonType(DataType left, DataType right)
    {
        if (isPointerType(left) && isPointerType(right))
        {
            if (left == right)
                return left;
            // void* is compatible with any pointer
            if (left == DataType::VOID_PTR)
                return right;
            if (right == DataType::VOID_PTR)
                return left;
            // Otherwise, they're incompatible - this might need special handling
            LOG_ERROR("Incompatible pointer types in expression");
            exit(EXIT_FAILURE);
        }

        // If one is pointer and one is integer, keep existing behavior
        // or add special handling as needed

        // Existing float/integer logic...
        if (isFloatType(left) || isFloatType(right))
        {
            if (isFloatType(left) && isFloatType(right))
            {
                return (getTypeSize(left) >= getTypeSize(right)) ? left : right;
            }
            else if (isFloatType(left))
            {
                return left;
            }
            else
            {
                return right;
            }
        }

        return (getTypeSize(left) >= getTypeSize(right)) ? left : right;
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

    // Type inference methods (updated for float support)
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

            // &value
            DataType operator()(const NodeTermAddressOf *term_aof) const
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var &var)
                                       { return var.name == term_aof->ident.value.value(); });
                if (it == gen->m_vars.cend())
                {
                    LOG_ERROR("Undeclared identifier {}", term_aof->ident.value.value());
                    exit(EXIT_FAILURE);
                }
                return getPointerType((*it).type);
            }

            // *ptr
            DataType operator()(const NodeTermDereference *term_deref) const
            {
                auto ptrType = gen->inferExpressionType(term_deref->expr);
                if (!isPointerType(ptrType))
                {
                    LOG_ERROR("Cannot dereference non-pointer type");
                    exit(EXIT_FAILURE);
                }
                return getPointeeType(ptrType);
            }

            // ptr[expr]
            DataType operator()(const NodeTermArrayAccess *array_access) const
            {
                DataType array_type = gen->inferExpressionType(array_access->array_expr);
                if (!isPointerType(array_type))
                {
                    LOG_ERROR("Cannot index non-pointer type");
                    exit(EXIT_FAILURE);
                }
                return getPointeeType(array_type);
            }

            DataType operator()(const NodeTermStringLiteral *term_str_lit) const
            {
                return DataType::INT8_PTR;
            }

            DataType operator()(const NodeTermIntegerLiteral *term_int_lit) const
            {
                return DataType::INT32;
            }

            DataType operator()(const NodeTermFloatLiteral *term_float_lit) const
            {
                return DataType::FLOAT32;
            }

            DataType operator()(const NodeTermDoubleLiteral *term_float_lit) const
            {
                return DataType::FLOAT64;
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

            DataType operator()(const NodeTermCast *term_cast) const
            {
                return term_cast->target_type;
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
                return gen->getCommonType(leftType, rightType);
            }
            DataType operator()(const NodeExpressionBinaryAddition *add) const
            {
                DataType leftType = gen->inferExpressionType(add->left);
                DataType rightType = gen->inferExpressionType(add->right);
                return gen->getCommonType(leftType, rightType);
            }
            DataType operator()(const NodeExpressionBinaryMultiplication *mul) const
            {
                DataType leftType = gen->inferExpressionType(mul->left);
                DataType rightType = gen->inferExpressionType(mul->right);
                return gen->getCommonType(leftType, rightType);
            }
            DataType operator()(const NodeExpressionBinaryDivision *div) const
            {
                DataType leftType = gen->inferExpressionType(div->left);
                DataType rightType = gen->inferExpressionType(div->right);
                return gen->getCommonType(leftType, rightType);
            }
            DataType operator()(const NodeExpressionBinaryGreater *) const
            {
                return DataType::INT32; // Comparisons always return boolean (represented as int32)
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
        // With automatic type conversion, we're more lenient
        // Only check for fundamentally incompatible types (e.g., void with non-void)
        if (expected == DataType::VOID && actual != DataType::VOID)
        {
            LOG_ERROR("Type mismatch in {}: expected void but got {}",
                      context, typeToString(actual));
            exit(EXIT_FAILURE);
        }
        if (expected != DataType::VOID && actual == DataType::VOID)
        {
            LOG_ERROR("Type mismatch in {}: expected {} but got void",
                      context, typeToString(expected));
            exit(EXIT_FAILURE);
        }
        // All other numeric types are convertible
    }

    Function *Assembler::findFunction(const std::string &name)
    {
        auto it = std::find_if(m_functions.begin(), m_functions.end(),
                               [&name](const Function &func)
                               { return func.name == name; });
        return (it != m_functions.end()) ? &(*it) : nullptr;
    }

    void Assembler::addFunction(const std::string &name,
                                const std::vector<DataType> &param_types,
                                DataType ret_type, bool external,
                                bool variadic)
    {
        Function func(name, param_types, ret_type, name, external, variadic);
        m_functions.emplace_back(func);
        m_used_external_functions.insert(name);
    }

    void
    Assembler::registerBuiltinFunctions()
    {
        // ---- C Standard ----
        addFunction("exit", {DataType::INT32}, DataType::VOID, true, false);
        addFunction("printf", {DataType::INT8_PTR}, DataType::INT32, true, true);
        addFunction("malloc", {DataType::INT64}, DataType::VOID_PTR, true, false);
        addFunction("free", {DataType::VOID_PTR}, DataType::VOID, true, false);
        addFunction("strlen", {DataType::INT8_PTR}, DataType::INT64, true, false);
        addFunction("strcpy", {DataType::INT8_PTR, DataType::INT8_PTR}, DataType::INT8_PTR, true, false);

        // ---- Delta Standard ----
        addFunction("helloWorld", {}, DataType::VOID, true, false);
    }

    void Assembler::registerExternalFunctions()
    {
        // None for now
    }

    void Assembler::validateFunctionCall(const std::string &func_name, const std::vector<NodeExpression *> &arguments)
    {
        Function *func = findFunction(func_name);
        if (!func)
        {
            LOG_ERROR("Unknown function: {}", func_name);
            exit(EXIT_FAILURE);
        }

        if (func->is_variadic)
        {
            if (arguments.size() < func->parameter_types.size())
            {
                LOG_ERROR("Variadic function {} requires at least {} arguments but got {}",
                          func_name, func->parameter_types.size(), arguments.size());
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (arguments.size() != func->parameter_types.size())
            {
                LOG_ERROR("Function {} expects {} arguments but got {}",
                          func_name, func->parameter_types.size(), arguments.size());
                exit(EXIT_FAILURE);
            }
        }

        for (size_t i = 0; i < func->parameter_types.size(); i++)
        {
            DataType argType = inferExpressionType(arguments[i]);
            DataType expectedType = func->parameter_types[i];

            if (argType == DataType::VOID || expectedType == DataType::VOID)
            {
                LOG_ERROR("Void type not allowed in function argument {} to function {}",
                          i + 1, func_name);
                exit(EXIT_FAILURE);
            }
        }

        if (func->is_variadic)
        {
            for (size_t i = func->parameter_types.size(); i < arguments.size(); i++)
            {
                DataType argType = inferExpressionType(arguments[i]);
                if (argType == DataType::VOID)
                {
                    LOG_ERROR("Void type not allowed in variadic argument {} to function {}",
                              i + 1, func_name);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    std::string Assembler::applyDefaultPromotions(const std::string &value, DataType &type)
    {
        DataType promoted_type = getPromotedType(type);

        if (type != promoted_type)
        {
            std::string converted_value = generateTypeConversion(value, type, promoted_type);
            type = promoted_type;
            return converted_value;
        }

        return value;
    }

    DataType Assembler::getPromotedType(DataType type)
    {
        switch (type)
        {
        case DataType::INT8:
        case DataType::INT16:
            return DataType::INT32;
        case DataType::FLOAT32:
            return DataType::FLOAT64;
        default:
            return type;
        }
    }

    std::string Assembler::escapeString(const std::string &str)
    {
        std::string result;
        for (char c : str)
        {
            switch (c)
            {
            case '\n':
                result += "\\0A";
                break;
            case '\t':
                result += "\\09";
                break;
            case '\r':
                result += "\\0D";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\22";
                break;
            default:
                if (c >= 32 && c <= 126)
                {
                    result += c;
                }
                else
                {
                    result += "\\";
                    result += std::to_string((unsigned char)c / 64);
                    result += std::to_string(((unsigned char)c / 8) % 8);
                    result += std::to_string((unsigned char)c % 8);
                }
                break;
            }
        }
        return result;
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

                if (func->is_variadic)
                {
                    if (!func->parameter_types.empty())
                        m_output << ", ";
                    m_output << "...";
                }

                m_output << ")\n";
            }
        }
        m_output << "\n";
    }

    void Assembler::collectStringLiterals()
    {
        for (const NodeFunctionDeclaration *func : m_program.functions)
        {
            collectStringLiteralsFromScope(func->body);
        }
    }

    void Assembler::collectStringLiteralsFromScope(const NodeScope *scope)
    {
        for (const NodeStatement *stmt : scope->statements)
        {
            collectStringLiteralsFromStatement(stmt);
        }
    }

    void Assembler::collectStringLiteralsFromStatement(const NodeStatement *statement)
    {
        struct StringCollectionStatementVisitor
        {
            Assembler *gen;
            StringCollectionStatementVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeExpression *expression)
            {
                gen->collectStringLiteralsFromExpression(expression);
            }

            void operator()(const NodeStatementExit *statement_exit)
            {
                gen->collectStringLiteralsFromExpression(statement_exit->expression);
            }

            void operator()(const NodeStatementLet *statement_let)
            {
                gen->collectStringLiteralsFromExpression(statement_let->expression);
            }

            void operator()(const NodeStatementAssign *assign)
            {
                gen->collectStringLiteralsFromExpression(assign->expression);
            }

            void operator()(const NodeScope *scope)
            {
                gen->collectStringLiteralsFromScope(scope);
            }

            void operator()(const NodeStatementIf *statement_if)
            {
                gen->collectStringLiteralsFromExpression(statement_if->expr);
                gen->collectStringLiteralsFromScope(statement_if->scope);

                if (statement_if->pred.has_value())
                {
                    gen->collectStringLiteralsFromIfPred(statement_if->pred.value());
                }
            }

            void operator()(const NodeStatementReturn *statement_return)
            {
                if (statement_return->expression)
                {
                    gen->collectStringLiteralsFromExpression(statement_return->expression);
                }
            }

            void operator()(const NodeStatementPointerAssign *ptr_assign)
            {
                gen->collectStringLiteralsFromExpression(ptr_assign->ptr_expr);
                gen->collectStringLiteralsFromExpression(ptr_assign->value_expr);
            }

            void operator()(const NodeStatementArrayAssign *array_assign)
            {
                gen->collectStringLiteralsFromExpression(array_assign->array_expr);
                gen->collectStringLiteralsFromExpression(array_assign->index_expr);
                gen->collectStringLiteralsFromExpression(array_assign->value_expr);
            }
        };

        StringCollectionStatementVisitor visitor(this);
        std::visit(visitor, statement->var);
    }

    void Assembler::collectStringLiteralsFromExpression(const NodeExpression *expression)
    {
        struct StringCollectionExpressionVisitor
        {
            Assembler *gen;
            StringCollectionExpressionVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeExpressionTerm *expression_term)
            {
                gen->collectStringLiteralsFromTerm(expression_term);
            }

            void operator()(const NodeExpressionBinary *expression_binary)
            {
                gen->collectStringLiteralsFromBinaryExpression(expression_binary);
            }
        };

        StringCollectionExpressionVisitor visitor(this);
        std::visit(visitor, expression->var);
    }

    void Assembler::collectStringLiteralsFromTerm(const NodeExpressionTerm *term)
    {
        struct StringCollectionTermVisitor
        {
            Assembler *gen;
            StringCollectionTermVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeTermStringLiteral *term_str_lit)
            {
                std::string str_value = term_str_lit->string_literal.value.value();

                // Add to collection if not already present
                auto it = std::find(gen->m_string_literals.begin(),
                                    gen->m_string_literals.end(), str_value);
                if (it == gen->m_string_literals.end())
                {
                    gen->m_string_literals.push_back(str_value);
                }
            }

            void operator()(const NodeTermIntegerLiteral *) { /* No strings here */ }
            void operator()(const NodeTermFloatLiteral *) { /* No strings here */ }
            void operator()(const NodeTermDoubleLiteral *) { /* No strings here */ }
            void operator()(const NodeTermIdentifier *) { /* No strings here */ }

            void operator()(const NodeTermParen *term_paren)
            {
                gen->collectStringLiteralsFromExpression(term_paren->expr);
            }

            void operator()(const NodeTermFunctionCall *func_call)
            {
                for (const NodeExpression *arg : func_call->arguments)
                {
                    gen->collectStringLiteralsFromExpression(arg);
                }
            }

            void operator()(const NodeTermCast *term_cast)
            {
                gen->collectStringLiteralsFromExpression(term_cast->expr);
            }

            void operator()(const NodeTermAddressOf *) { /* No strings here */ }

            void operator()(const NodeTermDereference *term_deref)
            {
                gen->collectStringLiteralsFromExpression(term_deref->expr);
            }

            void operator()(const NodeTermArrayAccess *array_access)
            {
                gen->collectStringLiteralsFromExpression(array_access->array_expr);
                gen->collectStringLiteralsFromExpression(array_access->index_expr);
            }
        };

        StringCollectionTermVisitor visitor(this);
        std::visit(visitor, term->var);
    }

    void Assembler::collectStringLiteralsFromBinaryExpression(const NodeExpressionBinary *bin_expr)
    {
        struct StringCollectionBinaryVisitor
        {
            Assembler *gen;
            StringCollectionBinaryVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeExpressionBinaryAddition *add)
            {
                gen->collectStringLiteralsFromExpression(add->left);
                gen->collectStringLiteralsFromExpression(add->right);
            }

            void operator()(const NodeExpressionBinarySubtraction *sub)
            {
                gen->collectStringLiteralsFromExpression(sub->left);
                gen->collectStringLiteralsFromExpression(sub->right);
            }

            void operator()(const NodeExpressionBinaryMultiplication *mul)
            {
                gen->collectStringLiteralsFromExpression(mul->left);
                gen->collectStringLiteralsFromExpression(mul->right);
            }

            void operator()(const NodeExpressionBinaryDivision *div)
            {
                gen->collectStringLiteralsFromExpression(div->left);
                gen->collectStringLiteralsFromExpression(div->right);
            }

            void operator()(const NodeExpressionBinaryGreater *gt)
            {
                gen->collectStringLiteralsFromExpression(gt->left);
                gen->collectStringLiteralsFromExpression(gt->right);
            }

            void operator()(const NodeExpressionBinaryGreaterEquals *gte)
            {
                gen->collectStringLiteralsFromExpression(gte->left);
                gen->collectStringLiteralsFromExpression(gte->right);
            }

            void operator()(const NodeExpressionBinaryLess *lt)
            {
                gen->collectStringLiteralsFromExpression(lt->left);
                gen->collectStringLiteralsFromExpression(lt->right);
            }

            void operator()(const NodeExpressionBinaryLessEquals *lte)
            {
                gen->collectStringLiteralsFromExpression(lte->left);
                gen->collectStringLiteralsFromExpression(lte->right);
            }

            void operator()(const NodeExpressionBinaryEquals *eq)
            {
                gen->collectStringLiteralsFromExpression(eq->left);
                gen->collectStringLiteralsFromExpression(eq->right);
            }
        };

        StringCollectionBinaryVisitor visitor(this);
        std::visit(visitor, bin_expr->var);
    }

    void Assembler::collectStringLiteralsFromIfPred(const NodeIfPred *pred)
    {
        struct StringCollectionPredVisitor
        {
            Assembler *gen;
            StringCollectionPredVisitor(Assembler *gen) : gen(gen) {}

            void operator()(const NodeIfPredElif *pred_elif)
            {
                gen->collectStringLiteralsFromExpression(pred_elif->expr);
                gen->collectStringLiteralsFromScope(pred_elif->scope);

                if (pred_elif->pred.has_value())
                {
                    gen->collectStringLiteralsFromIfPred(pred_elif->pred.value());
                }
            }

            void operator()(const NodeIfPredElse *pred_else)
            {
                gen->collectStringLiteralsFromScope(pred_else->scope);
            }
        };

        StringCollectionPredVisitor visitor(this);
        std::visit(visitor, pred->var);
    }

} // namespace Delta