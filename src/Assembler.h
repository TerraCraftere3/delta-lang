#pragma once

#include "Nodes.h"
#include "Types.h"
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <set>

// Updated Var struct for LLVM IR
struct Var
{
    std::string name;
    size_t stack_loc; // Keep for compatibility, but not used in LLVM IR
    Delta::DataType type;
    size_t type_size;
    bool isConstant = false;
    std::string llvm_alloca; // LLVM alloca instruction result (e.g., "%var1")

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
    std::string llvm_name;
    bool is_external;
    bool is_variadic; // any amount of variables, like printf(str, ...)

    Function(const std::string &name, const std::vector<Delta::DataType> &param_types,
             Delta::DataType ret_type, const std::string &llvm_name, bool external = false, bool variadic = false)
        : name(name), parameter_types(param_types), return_type(ret_type),
          llvm_name(llvm_name), is_external(external), is_variadic(variadic) {}
};

// Keep for potential future use or compatibility
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

        // Updated method signatures - now return values instead of using stack
        std::string generateTerm(const NodeExpressionTerm *term);
        std::string generateBinaryExpression(const NodeExpressionBinary *bin_expr);
        std::string generateExpression(const NodeExpression *expression);
        void generateScope(const NodeScope *scope);
        void generateIfPred(const NodeIfPred *pred, const std::string &merge_label);
        void generateStatement(const NodeStatement *statement);
        void generateStringLiterals();
        std::string generateFunctionCall(const NodeTermFunctionCall *func_call);
        void generateFunctionDeclaration(const NodeFunctionDeclaration *func_decl);

    private:
        // LLVM IR specific helpers
        std::string getNextTemp();                 // Generate next temporary (%t0, %t1, etc.)
        std::string getNextLabel();                // Generate next basic block label
        std::string dataTypeToLLVM(DataType type); // Convert DataType to LLVM IR type
        void generateDefaultValue(DataType type);  // Generate default value for type

        // Type conversion methods
        std::string generateTypeConversion(const std::string &value, DataType from, DataType to);
        std::string convertToBoolean(const std::string &value, DataType type);
        DataType getCommonType(DataType left, DataType right);

        // Module structure
        void declareFunctions();
        void generateExternDeclarations();

        // Strings
        void collectStringLiterals();
        void collectStringLiteralsFromScope(const NodeScope *scope);
        void collectStringLiteralsFromStatement(const NodeStatement *statement);
        void collectStringLiteralsFromExpression(const NodeExpression *expression);
        void collectStringLiteralsFromTerm(const NodeExpressionTerm *term);
        void collectStringLiteralsFromBinaryExpression(const NodeExpressionBinary *bin_expr);
        void collectStringLiteralsFromIfPred(const NodeIfPred *pred);

        // Scope and function management (simplified for LLVM)
        void begin_scope();
        void end_scope();
        void begin_function(const std::string &func_name);
        void end_function();

        // Type inference and validation (reused from original)
        DataType inferExpressionType(const NodeExpression *expression);
        DataType inferTermType(const NodeExpressionTerm *term);
        DataType inferBinaryExpressionType(const NodeExpressionBinary *bin_expr);
        void validateTypeCompatibility(DataType expected, DataType actual, const std::string &context);

        // Function management
        Function *findFunction(const std::string &name);
        void registerBuiltinFunctions();
        void registerExternalFunctions();
        void validateFunctionCall(const std::string &func_name, const std::vector<NodeExpression *> &arguments);
        std::string applyDefaultPromotions(const std::string &value, DataType &type);
        DataType getPromotedType(DataType type);

        std::string escapeString(const std::string &str);

    private:
        const NodeProgram m_program;
        std::stringstream m_output;

        // LLVM IR specific counters
        size_t m_current_block_id = 0; // For generating basic block labels
        size_t m_temp_counter = 0;     // For generating temporary variables

        std::vector<Var> m_vars{};
        std::vector<size_t> m_scopes{};
        std::vector<Function> m_functions{};
        std::set<std::string> m_used_external_functions{}; // Track which external functions are used
        std::vector<std::string> m_string_literals;

        // Function context
        std::string m_current_function = "";
        size_t m_function_param_count = 0; // Legacy, not used in LLVM version
        DataType m_current_function_return_type = DataType::VOID;
        bool m_in_function = false;

        // Type tracking for expressions
        std::map<const NodeExpression *, DataType> m_expression_types{};
    };
};