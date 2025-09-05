#include "Compiler.h"
#include "Log.h"

int Delta::Compiler::compile(const CompilerProperties &props)
{
    LOG_INFO("Compiling {}...", props.inputFile);
    if (props.verbose)
    {
        LOG_INFO("Output File: {}", props.outputFile);
        LOG_INFO("Compile Type: {}", getCompileTypeName(props.compileType));
    }
    return 0;
}