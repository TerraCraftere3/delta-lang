#pragma once

#include "Nodes.h"
#include "Types.h"
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <set>

// not used by anything else except Assembler.cpp
struct Var
{
    std::string name;
    size_t stack_loc;
    Delta::DataType type;
    size_t type_size;
    bool isConstant = false;

    Var(const std::string &n, size_t loc, Delta::DataType t)
        : name(n), stack_loc(loc), type(t), type_size(Delta::getTypeSize(t)) {}

    void setConstant(bool c)
    {
        isConstant = c;
    }
};

struct Function
{
    std::string name;
    std::vector<Delta::DataType> parameter_types;
    Delta::DataType return_type;
    std::string label;
    bool is_external = false;
    std::string library = ""; // Which library the function belongs to

    Function(const std::string &n, const std::vector<Delta::DataType> &params,
             Delta::DataType ret_type, const std::string &lbl = "", bool external = false, const std::string &lib = "")
        : name(n), parameter_types(params), return_type(ret_type), label(lbl), is_external(external), library(lib) {}
};

struct WindowsAPIFunction
{
    std::string name;
    std::vector<Delta::DataType> parameter_types;
    Delta::DataType return_type;
    std::string library;       // "kernel32" or "msvcrt"
    bool uses_stdcall = false; // Most Win32 API uses stdcall, C runtime uses cdecl
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
        void generateFunctionCall(const NodeTermFunctionCall *func_call);
        void generateFunctionDeclaration(const NodeFunctionDeclaration *func_decl);

    private:
        void push(const std::string &reg);
        void pop(const std::string &reg);
        void pushTyped(const std::string &reg, DataType type);
        void popTyped(const std::string &reg, DataType type);
        std::string create_label();
        std::string create_function_label(const std::string &func_name);
        void begin_scope();
        void end_scope();
        void begin_function(const std::string &func_name);
        void end_function();
        void alignStackAndCall(const std::string &function);
        void setupFunctionPrologue();
        void setupFunctionEpilogue();

        // Type-aware helper methods
        std::string getAppropriateRegister(DataType type, const std::string &base_reg = "rax");
        void generateTypedMove(const std::string &dest, const std::string &src, DataType type);
        void generateTypedBinaryOp(const std::string &op, DataType leftType, DataType rightType);
        DataType inferExpressionType(const NodeExpression *expression);
        DataType inferTermType(const NodeExpressionTerm *term);
        DataType inferBinaryExpressionType(const NodeExpressionBinary *bin_expr);
        void validateTypeCompatibility(DataType expected, DataType actual, const std::string &context);

        // Function-related helpers
        Function *findFunction(const std::string &name);
        void registerBuiltinFunctions();
        void validateFunctionCall(const std::string &func_name, const std::vector<NodeExpression *> &arguments);
        std::vector<std::string> getCallingConventionRegisters();
        void passArgumentsToFunction(const std::vector<NodeExpression *> &arguments, const Function &func);

        // Windows API support
        void registerWindowsAPIFunctions();
        void addWindowsAPIFunction(const std::string &name, const std::vector<DataType> &params,
                                   DataType return_type, const std::string &library, bool stdcall = false);
        bool isWindowsAPIFunction(const std::string &name);
        void generateWindowsAPICall(const std::string &func_name, const std::vector<NodeExpression *> &arguments);
        void generateExternDeclarations();
        void passArgumentsToWindowsAPI(const std::vector<NodeExpression *> &arguments, const Function &func);

    private:
        const NodeProgram m_program;
        std::stringstream m_output;
        size_t m_stack_size = 0;
        size_t m_stack_byte_size = 0;
        size_t m_label_count = 0;
        std::vector<Var> m_vars{};
        std::vector<size_t> m_scopes{};
        std::vector<Function> m_functions{};
        std::map<std::string, WindowsAPIFunction> m_windows_api_functions{};
        std::set<std::string> m_used_external_functions{}; // Track which external functions are actually used

        // Function context
        std::string m_current_function = "";
        size_t m_function_param_count = 0;
        DataType m_current_function_return_type = DataType::VOID;
        bool m_in_function = false;

        // Type tracking for expressions
        std::map<const NodeExpression *, DataType> m_expression_types{};
    };
}