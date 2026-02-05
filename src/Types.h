#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace Delta
{
    enum class BaseType
    {
        ERRORTYPE = 0,
        // Scalar
        INT8,
        INT16,
        INT32,
        INT64,
        FLOAT32,
        FLOAT64,
        VOID,
        STRUCT // reserved for future struct support
    };

    struct DataType
    {
        BaseType base;
        std::uint16_t pointer_level;
        std::string struct_name;

        constexpr DataType(BaseType b = BaseType::ERRORTYPE, std::uint16_t level = 0) : base(b), pointer_level(level) {}

        bool operator==(const DataType& other) const
        {
            return base == other.base && pointer_level == other.pointer_level;
        }
        bool operator!=(const DataType& other) const { return !(*this == other); }

        bool isPointer() const { return pointer_level > 0; }

        static DataType pointerTo(const DataType& type)
        {
            return DataType(type.base, static_cast<std::uint16_t>(type.pointer_level + 1));
        }

        // Convenience singletons for existing call sites
        static const DataType ERRORTYPE;
        static const DataType INT8;
        static const DataType INT16;
        static const DataType INT32;
        static const DataType INT64;
        static const DataType FLOAT32;
        static const DataType FLOAT64;
        static const DataType VOID;

        static const DataType INT8_PTR;
        static const DataType INT16_PTR;
        static const DataType INT32_PTR;
        static const DataType INT64_PTR;
        static const DataType FLOAT32_PTR;
        static const DataType FLOAT64_PTR;
        static const DataType VOID_PTR;
    };

    bool isFloatType(DataType type);
    bool isPointerType(DataType type);
    size_t getTypeSize(DataType type);
    size_t getTypeAlignment(DataType type);
    DataType getPointerType(DataType baseType);
    DataType getPointeeType(DataType ptrType);
    bool isTypeCompatible(DataType declared, DataType actual);
    std::string typeToString(DataType type);
    DataType stringToType(const std::string& s);
    bool isValidDataType(const std::string& s);
} // namespace Delta