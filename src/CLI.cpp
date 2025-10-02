#include <iostream>
#include <string>
#include "Log.h"
#include "Properties.h"
#include "Compiler.h"

int main(int argc, char **argv)
{
    std::string inputFile;
    std::string outputFile = "a.exe";
    bool verbose = true;
    bool link = false;
    bool run = false;

    std::vector<std::string> includeDirs;
    std::vector<std::string> linkerFiles;

    Delta::CompileTarget target = Delta::CompileTarget::TARGET_NATIVE;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            std::cout << "Delta Language Compiler\n";
            std::cout << "Usage: delta.exe [options]\n";
            std::cout << "Options:\n";
            std::cout << "\t-i, --input <file>         Specify input source file (required)\n";
            std::cout << "\t-o, --output <file>        Specify output executable file (default: a.exe)\n";
            std::cout << "\t-I, --include <dir>        Add an include directory (can be repeated)\n";
            std::cout << "\t-L, --link-lib <file>      Add a linker library/file (can be repeated)\n";
            std::cout << "\t-v, --verbose              Enable verbose logging (default)\n";
            std::cout << "\t--link                     Compile and link the program\n";
            std::cout << "\t-r, --run                  Compile, link, and run the program\n";
            std::cout << "\t-h, --help                 Show this help message\n";
            return 0;
        }
        else if (arg == "-i" || arg == "--input")
        {
            if (i + 1 < argc)
                inputFile = argv[++i];
            else
            {
                std::cerr << "Error: -i/--input requires a filename\n";
                return 1;
            }
        }
        else if (arg == "-o" || arg == "--output")
        {
            if (i + 1 < argc)
                outputFile = argv[++i];
            else
            {
                std::cerr << "Error: -o/--output requires a filename\n";
                return 1;
            }
        }
        else if (arg == "-I" || arg == "--include")
        {
            if (i + 1 < argc)
                includeDirs.push_back(argv[++i]);
            else
            {
                std::cerr << "Error: -I/--include requires a directory\n";
                return 1;
            }
        }
        else if (arg == "-L" || arg == "--link-lib")
        {
            if (i + 1 < argc)
                linkerFiles.push_back(argv[++i]);
            else
            {
                std::cerr << "Error: -L/--link-lib requires a library/file\n";
                return 1;
            }
        }
        else if (arg == "-T" || arg == "--target")
        {
            if (i + 1 < argc)
            {
                if (std::string(argv[++i]) == "native")
                {
                    target = Delta::CompileTarget::TARGET_NATIVE;
                }
                else if (std::string(argv[i]) == "wasm")
                {
                    target = Delta::CompileTarget::TARGET_WASM;
                }
                else
                {
                    std::cerr << "Error: Invalid target '" << argv[i] << "'. Use 'native' or 'wasm'.\n";
                    return 1;
                }
            }
            else
            {
                std::cerr << "Error: -T/--target requires a target\n";
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
        else if (arg == "--link")
        {
            link = true;
        }
        else
        {
            std::cerr << "Unknown argument: \"" << arg << "\"\n";
            return 1;
        }
    }

    if (inputFile.empty())
    {
        std::cerr << "Error: No input file specified. Use -i or --input to specify an input file.";
        return 1;
    }
    Delta::Log::init(inputFile + ".log");
    Delta::CompilerProperties props;
    props.inputFile = inputFile.c_str();
    props.outputFile = outputFile.c_str();
    props.verbose = verbose;
    props.compileType = run ? Delta::COMPILE_LINK_AND_RUN : (link ? Delta::COMPILE_AND_LINK : Delta::COMPILE_ONLY);
    props.additionalLinks = linkerFiles;
    props.includeDirs = includeDirs;
    props.compileTarget = target;
    return Delta::Compiler::compile(props);
}