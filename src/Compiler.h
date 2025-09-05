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
        static std::vector<Token> tokenize(const std::string &source);
        static std::string assemble(const std::vector<Token> &tokens);
    };
}