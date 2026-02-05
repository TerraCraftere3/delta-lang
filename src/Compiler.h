#pragma once

#include <string>
#include <vector>
#include "Properties.h"
#include "Tokens.h"

namespace Delta
{
    class Compiler
    {
    public:
        static bool openInBrowser(const std::string& htmlPath);
        static bool generateWasmHtml(const std::string& htmlPath, const std::string& wasmFile);
        static int compile(const CompilerProperties& props);
    };
} // namespace Delta