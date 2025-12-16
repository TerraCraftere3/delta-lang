#pragma once

#include <vector>
#include <string>

namespace Delta
{
    enum CompileType
    {
        COMPILE_ONLY,
        COMPILE_AND_LINK,
        COMPILE_LINK_AND_RUN
    };

    enum CompileTarget
    {
        TARGET_NATIVE,
        TARGET_WASM
    };

    inline const char *getCompileTypeName(CompileType type)
    {
        switch (type)
        {
        case COMPILE_ONLY:
            return "Compile Only";
        case COMPILE_AND_LINK:
            return "Compile and Link";
        case COMPILE_LINK_AND_RUN:
            return "Compile, Link and Run";
        default:
            return "Unknown";
        }
    }

    struct CompilerProperties
    {
        bool verbose = false;
        CompileType compileType = COMPILE_ONLY;
        CompileTarget compileTarget = TARGET_NATIVE;
        std::vector<std::string> inputFiles; // support multiple input files
        const char *outputFile = nullptr;
        std::vector<std::string> additionalLinks;
        std::vector<std::string> includeDirs;
    };
}