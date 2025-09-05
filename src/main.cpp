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
        if (arg == "-h" || arg == "--help")
        {
            std::cout << "Delta Language Compiler\n";
            std::cout << "Usage: delta.exe [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -i, --input <file>       Specify input source file (required)\n";
            std::cout << "  -o, --output <file>      Specify output executable file (default: a.exe)\n";
            std::cout << "  -v, --verbose            Enable verbose logging (default)\n";
            std::cout << "  -l, --link               Compile and link the program\n";
            std::cout << "  -r, --run                Compile, link, and run the program\n";
            std::cout << "  -h, --help               Show this help message\n";
            return 0;
        }
        else if (arg == "-i" || arg == "--input")
        {
            if (i + 1 < argc)
            {
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
            {
                outputFile = argv[++i];
            }
            else
            {
                LOG_ERROR("Error: -o/--output requires a filename");
                return 1;
            }
        }
        else if (arg == "-v" || arg == "--verbose")
        {
            verbose = true;
        }
        else if (arg == "-r" || arg == "--run")
        {
            run = true;
        }
        else if (arg == "-l" || arg == "--link")
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