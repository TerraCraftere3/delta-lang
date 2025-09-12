#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../src/Types.h"

namespace Delta
{
    class StdlibFunctions
    {
    public:
        void setAddFunction(const std::function<void(const std::string &, const std::vector<DataType> &, DataType, bool, bool)> &func)
        {
            m_addFunc = func;
        }

        void setupFunctions()
        {
            if (!m_addFunc)
            {
                return;
            }
            m_addFunc("stdOpenWindow", {DataType::INT8_PTR, DataType::INT32, DataType::INT32}, DataType::INT32, true, false);
            m_addFunc("stdIsWindowOpen", {DataType::INT32}, DataType::INT8, true, false);
            m_addFunc("stdKeepWindowOpen", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdUpdateWindow", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdDestroyWindow", {DataType::INT32}, DataType::VOID, true, false);
        }

    private:
        std::function<void(const std::string &, const std::vector<DataType> &, DataType, bool, bool)> m_addFunc;
    };
}