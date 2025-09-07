#pragma once

#include <string>

namespace Delta
{
    class Error
    {
    public:
        static void throwExpected(const std::string &c, int line, int col = 0);
    };
}