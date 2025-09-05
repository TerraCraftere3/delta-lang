#pragma once

#include "Properties.h"

namespace Delta
{
    class Compiler
    {
    public:
        static int compile(const CompilerProperties &props);
    };
}