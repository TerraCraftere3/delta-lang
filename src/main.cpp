#include <iostream>
#include <string>
#include "Log.h"
#include "Properties.h"
#include "Compiler.h"

int main(int argc, char **argv)
{
    Delta::Log::init();
    std::string inputFile;
    std::string outputFile = "a.exe";
    bool verbose = true;
    bool link = false;
    bool run = false;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--input")
        {
            if (i + 1 < argc)
            { // check next argument exists
                inputFile = argv[++i];
            }
            else
            {
                LOG_ERROR("Error: -i/--input requires a filename");
                return 1;
            }
        }
        else if (arg == "-o" || arg == "--output")
        {
            if (i + 1 < argc)
            { // check next argument exists
                outputFile = argv[++i];
            }
            else
            {
                LOG_ERROR("Error: -o/--output requires a filename");
                return 1;
            }
        }
        else if (arg == "--verbose" || arg == "-v")
        {
            verbose = true;
        }
        else if (arg == "--run" || arg == "-r")
        {
            run = true;
        }
        else if (arg == "--link" || arg == "-l")
        {
            link = true;
        }
        else
        {
            LOG_ERROR("Unknown argument: {}", arg);
            return 1;
        }
    }
    if (inputFile.empty())
    {
        LOG_ERROR("Error: No input file specified. Use -i or --input to specify an input file.");
        return 1;
    }
    Delta::CompilerProperties props;
    props.inputFile = inputFile.c_str();
    props.outputFile = outputFile.c_str();
    props.verbose = verbose;
    props.compileType = run ? Delta::COMPILE_LINK_AND_RUN : (link ? Delta::COMPILE_AND_LINK : Delta::COMPILE_ONLY);
    return Delta::Compiler::compile(props);
}