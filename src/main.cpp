#include <iostream>
#include <string>
#include "Log.h"

int main(int argc, char **argv)
{
    std::string inputFile;
    std::string outputFile = "a.exe";
    bool verbose = true;
    bool run = true;
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
        else
        {
            LOG_ERROR("Unknown argument: {}", arg);
            return 1;
        }
    }
    LOG_INFO("Input File: {}", inputFile);
    LOG_INFO("Output File: {}", outputFile);
    LOG_INFO("Verbose: {}", verbose ? "true" : "false");
    LOG_INFO("Run after Build: {}", run ? "true" : "false");
    return 0;
}