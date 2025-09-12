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
            // Standard Graphics
            m_addFunc("stdOpenWindow", {DataType::INT8_PTR, DataType::INT32, DataType::INT32}, DataType::INT32, true, false);
            m_addFunc("stdIsWindowOpen", {DataType::INT32}, DataType::INT8, true, false);
            m_addFunc("stdKeepWindowOpen", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdUpdateWindow", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdDestroyWindow", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdCloseWindow", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdMaximizeWindow", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdMinimizeWindow", {DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdIsKeyPressed", {DataType::INT32, DataType::INT8}, DataType::INT8, true, false);
            m_addFunc("stdClearWindow", {DataType::INT32, DataType::INT32, DataType::INT32, DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdSetWindowTitle", {DataType::INT32, DataType::INT8_PTR}, DataType::VOID, true, false);
            m_addFunc("stdSetWindowSize", {DataType::INT32, DataType::INT32, DataType::INT32}, DataType::VOID, true, false);
            m_addFunc("stdGetWindowSize", {DataType::INT32, DataType::INT32_PTR, DataType::INT32_PTR}, DataType::VOID, true, false);

            // Standard Time
            m_addFunc("stdSleep", {DataType::INT32}, DataType::VOID, true, false);
        }

    private:
        std::function<void(const std::string &, const std::vector<DataType> &, DataType, bool, bool)> m_addFunc;
    };
}