#include "Types.h"

namespace Delta
{
    size_t getTypeSize(DataType type)
    {
        switch (type)
        {
        case DataType::VOID:
            return 0;
        case DataType::INT8:
            return 1;
        case DataType::INT16:
            return 2;
        case DataType::INT32:
            return 4;
        case DataType::INT64:
            return 8;
        default:
            return 8;
        }
    }

    std::string getRegisterName(DataType type, const std::string &base_reg)
    {
        auto mapRegister = [&](const std::string &reg64,
                               const std::string &reg32,
                               const std::string &reg16,
                               const std::string &reg8l,
                               const std::string &reg8h = "") -> std::string
        {
            switch (type)
            {
            case DataType::INT8:
                return (!reg8h.empty() ? reg8l : reg8l);
            case DataType::INT16:
                return reg16;
            case DataType::INT32:
                return reg32;
            case DataType::INT64:
            default:
                return reg64;
            }
        };

        if (base_reg == "rax")
            return mapRegister("rax", "eax", "ax", "al", "ah");
        if (base_reg == "rbx")
            return mapRegister("rbx", "ebx", "bx", "bl", "bh");
        if (base_reg == "rcx")
            return mapRegister("rcx", "ecx", "cx", "cl", "ch");
        if (base_reg == "rdx")
            return mapRegister("rdx", "edx", "dx", "dl", "dh");

        if (base_reg == "rsi")
            return mapRegister("rsi", "esi", "si", "sil");
        if (base_reg == "rdi")
            return mapRegister("rdi", "edi", "di", "dil");
        if (base_reg == "rbp")
            return mapRegister("rbp", "ebp", "bp", "bpl");
        if (base_reg == "rsp")
            return mapRegister("rsp", "esp", "sp", "spl");

        if (base_reg == "r8")
            return mapRegister("r8", "r8d", "r8w", "r8b");
        if (base_reg == "r9")
            return mapRegister("r9", "r9d", "r9w", "r9b");
        if (base_reg == "r10")
            return mapRegister("r10", "r10d", "r10w", "r10b");
        if (base_reg == "r11")
            return mapRegister("r11", "r11d", "r11w", "r11b");
        if (base_reg == "r12")
            return mapRegister("r12", "r12d", "r12w", "r12b");
        if (base_reg == "r13")
            return mapRegister("r13", "r13d", "r13w", "r13b");
        if (base_reg == "r14")
            return mapRegister("r14", "r14d", "r14w", "r14b");
        if (base_reg == "r15")
            return mapRegister("r15", "r15d", "r15w", "r15b");

        return base_reg;
    }

    bool isTypeCompatible(DataType declared, DataType actual)
    {
        if (declared == actual)
            return true;

        if (declared == DataType::VOID || actual == DataType::VOID)
            return false;

        if ((declared == DataType::INT8 || declared == DataType::INT16 ||
             declared == DataType::INT32 || declared == DataType::INT64) &&
            (actual == DataType::INT8 || actual == DataType::INT16 ||
             actual == DataType::INT32 || actual == DataType::INT64))
        {
            return true;
        }

        return false;
    }

    std::string typeToString(DataType type)
    {
        switch (type)
        {
        case DataType::VOID:
            return "void";
        case DataType::INT8:
            return "int8";
        case DataType::INT16:
            return "int16";
        case DataType::INT32:
            return "int32";
        case DataType::INT64:
            return "int64";
        default:
            return "<errortype>";
        }
    }

    DataType stringToType(std::string s)
    {
        if (s == "void")
            return DataType::VOID;
        else if (s == "int8")
            return DataType::INT8;
        else if (s == "short")
            return DataType::INT16;
        else if (s == "int16")
            return DataType::INT16;
        else if (s == "int")
            return DataType::INT32;
        else if (s == "int32")
            return DataType::INT32;
        else if (s == "long")
            return DataType::INT64;
        else if (s == "int64")
            return DataType::INT64;
        return DataType::ERRORTYPE;
    }

    bool isValidDataType(std::string s)
    {
        auto type = stringToType(s);
        return type != DataType::ERRORTYPE;
    }
} // namespace Delta