#pragma once

#include "Properties.h"
#include "Tokens.h"
#include <string>
#include <vector>

namespace Delta
{
    class Compiler
    {
    public:
        static int compile(const CompilerProperties &props);
    };
}