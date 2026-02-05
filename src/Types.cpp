#include "Types.h"
#include "Error.h"
#include "Log.h"

namespace Delta
{
    const DataType DataType::ERRORTYPE = DataType(BaseType::ERRORTYPE, 0);
    const DataType DataType::INT8 = DataType(BaseType::INT8, 0);
    const DataType DataType::INT16 = DataType(BaseType::INT16, 0);
    const DataType DataType::INT32 = DataType(BaseType::INT32, 0);
    const DataType DataType::INT64 = DataType(BaseType::INT64, 0);
    const DataType DataType::FLOAT32 = DataType(BaseType::FLOAT32, 0);
    const DataType DataType::FLOAT64 = DataType(BaseType::FLOAT64, 0);
    const DataType DataType::VOID = DataType(BaseType::VOID, 0);

    const DataType DataType::INT8_PTR = DataType(BaseType::INT8, 1);
    const DataType DataType::INT16_PTR = DataType(BaseType::INT16, 1);
    const DataType DataType::INT32_PTR = DataType(BaseType::INT32, 1);
    const DataType DataType::INT64_PTR = DataType(BaseType::INT64, 1);
    const DataType DataType::FLOAT32_PTR = DataType(BaseType::FLOAT32, 1);
    const DataType DataType::FLOAT64_PTR = DataType(BaseType::FLOAT64, 1);
    const DataType DataType::VOID_PTR = DataType(BaseType::VOID, 1);

    namespace
    {
        std::string baseTypeToString(BaseType base)
        {
            switch (base)
            {
                case BaseType::VOID:
                    return "void";
                case BaseType::INT8:
                    return "int8";
                case BaseType::INT16:
                    return "int16";
                case BaseType::INT32:
                    return "int32";
                case BaseType::INT64:
                    return "int64";
                case BaseType::FLOAT32:
                    return "float32";
                case BaseType::FLOAT64:
                    return "float64";
                case BaseType::STRUCT:
                    return "struct";
                default:
                    return "<errortype>";
            }
        }

        BaseType stringToBase(const std::string& base)
        {
            if (base == "void")
                return BaseType::VOID;
            if (base == "char" || base == "int8")
                return BaseType::INT8;
            if (base == "short" || base == "int16")
                return BaseType::INT16;
            if (base == "int" || base == "int32")
                return BaseType::INT32;
            if (base == "long" || base == "int64")
                return BaseType::INT64;
            if (base == "float" || base == "float32")
                return BaseType::FLOAT32;
            if (base == "double" || base == "float64")
                return BaseType::FLOAT64;
            if (base == "struct")
                return BaseType::STRUCT;
            return BaseType::ERRORTYPE;
        }
    } // namespace

    size_t getTypeSize(DataType type)
    {
        if (type.pointer_level > 0)
            return 8; // 64-bit pointers

        switch (type.base)
        {
            case BaseType::INT8:
                return 1;
            case BaseType::INT16:
                return 2;
            case BaseType::INT32:
                return 4;
            case BaseType::INT64:
                return 8;
            case BaseType::FLOAT32:
                return 4;
            case BaseType::FLOAT64:
                return 8;
            case BaseType::VOID:
                return 0;
            default:
                return 0;
        }
    }

    size_t getTypeAlignment(DataType type)
    {
        if (type.pointer_level > 0)
            return 8;

        switch (type.base)
        {
            case BaseType::INT8:
                return 1;
            case BaseType::INT16:
                return 2;
            case BaseType::INT32:
                return 4;
            case BaseType::INT64:
                return 8;
            case BaseType::FLOAT32:
                return 4;
            case BaseType::FLOAT64:
                return 8;
            case BaseType::VOID:
                return 1;
            default:
                return 1;
        }
    }

    DataType getPointerType(DataType baseType)
    {
        if (baseType.base == BaseType::ERRORTYPE)
        {
            LOG_ERROR("Cannot create pointer to error type");
            BREAKPOINT();
            exit(EXIT_FAILURE);
        }
        return DataType::pointerTo(baseType);
    }

    DataType getPointeeType(DataType ptrType)
    {
        if (ptrType.pointer_level == 0)
        {
            LOG_ERROR("Not a pointer type");
            BREAKPOINT();
            exit(EXIT_FAILURE);
        }
        return DataType(ptrType.base, static_cast<std::uint16_t>(ptrType.pointer_level - 1));
    }

    bool isFloatType(DataType type)
    {
        return type.pointer_level == 0 && (type.base == BaseType::FLOAT32 || type.base == BaseType::FLOAT64);
    }

    bool isPointerType(DataType type)
    {
        return type.pointer_level > 0;
    }

    bool isTypeCompatible(DataType declared, DataType actual)
    {
        if (declared == actual)
            return true;

        if (declared.pointer_level > 0 || actual.pointer_level > 0)
        {
            if (declared.pointer_level == 0 || actual.pointer_level == 0)
                return false;

            if (declared.pointer_level != actual.pointer_level)
                return false;

            if (declared.base == BaseType::VOID || actual.base == BaseType::VOID)
                return true;

            return declared.base == actual.base;
        }

        if (declared.base == BaseType::VOID || actual.base == BaseType::VOID)
            return false;

        if ((declared.base == BaseType::INT8 || declared.base == BaseType::INT16 || declared.base == BaseType::INT32 ||
             declared.base == BaseType::INT64) &&
            (actual.base == BaseType::INT8 || actual.base == BaseType::INT16 || actual.base == BaseType::INT32 ||
             actual.base == BaseType::INT64))
        {
            return true;
        }

        return false;
    }

    std::string typeToString(DataType type)
    {
        std::string base = baseTypeToString(type.base);
        base.append(type.pointer_level, '*');
        return base;
    }

    DataType stringToType(const std::string& s)
    {
        if (s.empty())
            return DataType::ERRORTYPE;

        std::size_t first_star = s.find('*');
        std::string base_part = (first_star == std::string::npos) ? s : s.substr(0, first_star);
        std::uint16_t pointer_level = 0;
        if (first_star != std::string::npos)
        {
            for (std::size_t i = first_star; i < s.size(); ++i)
            {
                if (s[i] != '*')
                    return DataType::ERRORTYPE;
                pointer_level++;
            }
        }

        BaseType base = stringToBase(base_part);
        if (base == BaseType::ERRORTYPE)
            return DataType::ERRORTYPE;

        return DataType(base, pointer_level);
    }

    bool isValidDataType(const std::string& s)
    {
        DataType type = stringToType(s);
        return type != DataType::ERRORTYPE;
    }
} // namespace Delta