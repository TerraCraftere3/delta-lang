#include "Types.h"
#include "Log.h"
#include "Error.h"

namespace Delta
{
    size_t getTypeSize(DataType type)
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
        case DataType::FLOAT32:
            return 4;
        case DataType::FLOAT64:
            return 8;
        case DataType::INT8_PTR:
        case DataType::INT16_PTR:
        case DataType::INT32_PTR:
        case DataType::INT64_PTR:
        case DataType::FLOAT32_PTR:
        case DataType::FLOAT64_PTR:
        case DataType::VOID_PTR:
            return 8;
        default:
            return 4;
        }
    }

    size_t getTypeAlignment(DataType type)
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
        case DataType::FLOAT32:
            return 4;
        case DataType::FLOAT64:
            return 8;
        // All pointers are 8 bytes on 64-bit systems
        case DataType::INT8_PTR:
        case DataType::INT16_PTR:
        case DataType::INT32_PTR:
        case DataType::INT64_PTR:
        case DataType::FLOAT32_PTR:
        case DataType::FLOAT64_PTR:
        case DataType::VOID_PTR:
            return 8;
        default:
            return 4;
        }
    }

    DataType getPointerType(DataType baseType)
    {
        switch (baseType)
        {
        case DataType::INT8:
            return DataType::INT8_PTR;
        case DataType::INT16:
            return DataType::INT16_PTR;
        case DataType::INT32:
            return DataType::INT32_PTR;
        case DataType::INT64:
            return DataType::INT64_PTR;
        case DataType::FLOAT32:
            return DataType::FLOAT32_PTR;
        case DataType::FLOAT64:
            return DataType::FLOAT64_PTR;
        case DataType::VOID:
            return DataType::VOID_PTR;
        default:
            LOG_ERROR("Cannot create pointer to type");
            BREAKPOINT();
            exit(EXIT_FAILURE);
        }
    }

    DataType getPointeeType(DataType ptrType)
    {
        switch (ptrType)
        {
        case DataType::INT8_PTR:
            return DataType::INT8;
        case DataType::INT16_PTR:
            return DataType::INT16;
        case DataType::INT32_PTR:
            return DataType::INT32;
        case DataType::INT64_PTR:
            return DataType::INT64;
        case DataType::FLOAT32_PTR:
            return DataType::FLOAT32;
        case DataType::FLOAT64_PTR:
            return DataType::FLOAT64;
        case DataType::VOID_PTR:
            return DataType::VOID;
        default:
            LOG_ERROR("Not a pointer type");
            BREAKPOINT();
            exit(EXIT_FAILURE);
        }
    }

    bool isFloatType(DataType type)
    {
        return type == DataType::FLOAT32 || type == DataType::FLOAT64;
    }

    bool isPointerType(DataType type)
    {
        return type >= DataType::INT8_PTR && type <= DataType::VOID_PTR;
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
        case DataType::FLOAT32:
            return "float32";
        case DataType::FLOAT64:
            return "float64";

        // Pointer types
        case DataType::INT8_PTR:
            return "int8*";
        case DataType::INT16_PTR:
            return "int16*";
        case DataType::INT32_PTR:
            return "int32*";
        case DataType::INT64_PTR:
            return "int64*";
        case DataType::FLOAT32_PTR:
            return "float32*";
        case DataType::FLOAT64_PTR:
            return "float64*";
        case DataType::VOID_PTR:
            return "void*";

        default:
            return "<errortype>";
        }
    }

    DataType stringToType(const std::string &s)
    {
        if (s == "void")
            return DataType::VOID;
        else if (s == "char" || s == "int8")
            return DataType::INT8;
        else if (s == "short" || s == "int16")
            return DataType::INT16;
        else if (s == "int" || s == "int32")
            return DataType::INT32;
        else if (s == "long" || s == "int64")
            return DataType::INT64;
        else if (s == "float" || s == "float32")
            return DataType::FLOAT32;
        else if (s == "double" || s == "float64")
            return DataType::FLOAT64;

        // Pointer types
        else if (s == "char*" || s == "int8*")
            return DataType::INT8_PTR;
        else if (s == "short*" || s == "int16*")
            return DataType::INT16_PTR;
        else if (s == "int*" || s == "int32*")
            return DataType::INT32_PTR;
        else if (s == "long*" || s == "int64*")
            return DataType::INT64_PTR;
        else if (s == "float*" || s == "float32*")
            return DataType::FLOAT32_PTR;
        else if (s == "double*" || s == "float64*")
            return DataType::FLOAT64_PTR;
        else if (s == "void*")
            return DataType::VOID_PTR;

        return DataType::ERRORTYPE;
    }

    bool isValidDataType(const std::string &s)
    {
        DataType type = stringToType(s);
        return type != DataType::ERRORTYPE;
    }
} // namespace Delta