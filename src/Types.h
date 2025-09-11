#pragma once

#include "string"

namespace Delta
{
    enum class DataType
    {
        ERRORTYPE = 0,
        INT8,
        INT16,
        INT32,
        INT64,
        FLOAT32,
        FLOAT64,
        VOID,
    };

    bool isFloatType(DataType type);
    size_t getTypeSize(DataType type);
    bool isTypeCompatible(DataType declared, DataType actual);
    std::string typeToString(DataType type);
    DataType stringToType(std::string s);
    bool isValidDataType(std::string s);
}