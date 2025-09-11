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
        case DataType::FLOAT32:
        case DataType::INT32:
            return 4;
        case DataType::FLOAT64:
        case DataType::INT64:
            return 8;
        default:
            return 8;
        }
    }

    bool isFloatType(DataType type)
    {
        return type == DataType::FLOAT32 || type == DataType::FLOAT64;
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
            return "float32";
        default:
            return "<errortype>";
        }
    }

    DataType stringToType(std::string s)
    {
        if (s == "void")
            return DataType::VOID;
        else if (s == "char")
            return DataType::INT8;
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
        else if (s == "float")
            return DataType::FLOAT32;
        else if (s == "float32")
            return DataType::FLOAT32;
        else if (s == "double")
            return DataType::FLOAT64;
        else if (s == "float64")
            return DataType::FLOAT64;
        return DataType::ERRORTYPE;
    }

    bool isValidDataType(std::string s)
    {
        auto type = stringToType(s);
        return type != DataType::ERRORTYPE;
    }
} // namespace Delta