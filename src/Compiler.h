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
        static bool openInBrowser(const std::string &htmlPath);
        static bool generateWasmHtml(const std::string &htmlPath, const std::string &wasmFile);
        static int compile(const CompilerProperties &props);
    };
}