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
    };

    size_t getTypeSize(DataType type);
    std::string getRegisterName(DataType type, const std::string &base_reg);
    bool isTypeCompatible(DataType declared, DataType actual);
    std::string typeToString(DataType type);
    DataType stringToType(std::string s);
    bool isValidDataType(std::string s);
}