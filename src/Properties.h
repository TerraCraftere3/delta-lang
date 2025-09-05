#pragma once

namespace Delta
{
    enum CompileType
    {
        COMPILE_ONLY,
        COMPILE_AND_LINK,
        COMPILE_LINK_AND_RUN
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
        const char *inputFile = nullptr;
        const char *outputFile = nullptr;
    };
}