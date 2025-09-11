#pragma once

#include "string"

namespace Delta
{
    enum class DataType
    {
        ERRORTYPE = 0,
        // Normal
        INT8,
        INT16,
        INT32,
        INT64,
        FLOAT32,
        FLOAT64,
        VOID,
        // Pointer
        INT8_PTR,    // char*
        INT16_PTR,   // short*
        INT32_PTR,   // int*
        INT64_PTR,   // long*
        FLOAT32_PTR, // float*
        FLOAT64_PTR, // double*
        VOID_PTR     // void*
    };

    bool isFloatType(DataType type);
    bool isPointerType(DataType type);
    size_t getTypeSize(DataType type);
    size_t getTypeAlignment(DataType type);
    DataType getPointerType(DataType baseType);
    DataType getPointeeType(DataType ptrType);
    bool isTypeCompatible(DataType declared, DataType actual);
    std::string typeToString(DataType type);
    DataType stringToType(const std::string &s);
    bool isValidDataType(const std::string &s);
}