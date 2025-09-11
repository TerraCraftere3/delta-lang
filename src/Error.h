#pragma once

#include <string>

namespace Delta
{
#ifdef _MSC_VER
#define BREAKPOINT() __debugbreak()
#else
#include <csignal>
#define BREAKPOINT() std::raise(SIGTRAP)
#endif

    class Error
    {
    public:
        static void throwExpected(const std::string &c, int line, int col = 0);
    };
}