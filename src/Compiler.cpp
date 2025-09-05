#include "Compiler.h"
#include "Log.h"
#include "Files.h"
#include <fstream>

namespace Delta
{

    int Compiler::compile(const CompilerProperties &props)
    {
        Log::setVerbose(props.verbose);
        LOG_INFO("Compiling {}...", props.inputFile);
        LOG_TRACE("Compile Type: {}", getCompileTypeName(props.compileType));

        if (!Files::fileExists(props.inputFile))
        {
            LOG_ERROR("Input file does not exist: {}", props.inputFile);
            return 1;
        }

        std::string programPath = Files::getDirectory(Files::getProgramPath());
        std::string nasmPath = Files::joinPaths(programPath, "nasm.exe");
        if (!Files::fileExists(nasmPath))
        {
            LOG_ERROR("Nasm executable not found at: {}", nasmPath);
            return 1;
        }

        std::string linkPath = Files::joinPaths(programPath, "lld-link.exe");
        if (!Files::fileExists(linkPath))
        {
            LOG_ERROR("Linker executable not found at: {}", linkPath);
            return 1;
        }

        std::string contents = Files::readFile(props.inputFile);
        std::vector<Token> tokens = tokenize(contents);
        LOG_TRACE("Tokenized {} tokens", tokens.size());
        std::string assembly = assemble(tokens);

        std::string intDir = Files::joinPaths(Files::getDirectory(props.outputFile), "int");
        if (!Files::fileExists(intDir))
        {
            if (!Files::createDirectory(intDir))
            {
                LOG_ERROR("Failed to create intermediate directory: {}", intDir);
                return 1;
            }
        }
        switch (props.compileType)
        {
        case COMPILE_ONLY:
        {
            if (!Files::writeFile(props.outputFile, assembly))
            {
                LOG_ERROR("Failed to write output file: {}", props.outputFile);
                return 1;
            }
            break;
        }
        case COMPILE_AND_LINK:
        {
            std::string asmFile = Files::joinPaths(intDir, Files::getFileName(props.inputFile) + ".s");
            if (!Files::writeFile(asmFile, assembly))
            {
                LOG_ERROR("Failed to write intermediate file: {}", asmFile);
                return 1;
            }
            std::string objFile = Files::joinPaths(intDir, Files::getFileName(props.inputFile) + ".o");
            std::string command = "\"" + nasmPath + "\" -f win64 " + asmFile + " -o " + objFile + "";
            LOG_TRACE("Running command: {}", command);
            if (std::system(command.c_str()) != 0)
            {
                LOG_ERROR("Failed to execute command: {}", command);
                return 1;
            }
            std::string linkCommand = "\"" + linkPath + "\" " + objFile + " /subsystem:console /entry:_start /out:" + props.outputFile + " /defaultlib:kernel32.lib /defaultlib:msvcrt.lib";
            LOG_TRACE("Running command: {}", linkCommand);
            if (std::system(linkCommand.c_str()) != 0)
            {
                LOG_ERROR("Failed to execute command: {}", linkCommand);
                return 1;
            }
            break;
        }
        case COMPILE_LINK_AND_RUN:
        {
            std::string asmFile = Files::joinPaths(intDir, Files::getFileName(props.inputFile) + ".s");
            if (!Files::writeFile(asmFile, assembly))
            {
                LOG_ERROR("Failed to write intermediate file: {}", asmFile);
                return 1;
            }
            std::string objFile = Files::joinPaths(intDir, Files::getFileName(props.inputFile) + ".o");
            std::string command = "\"" + nasmPath + "\" -f win64 " + asmFile + " -o " + objFile + "";
            LOG_TRACE("Running command: {}", command);
            if (std::system(command.c_str()) != 0)
            {
                LOG_ERROR("Failed to execute command: {}", command);
                return 1;
            }
            std::string linkCommand = "\"" + linkPath + "\" /entry:_start /subsystem:console " + objFile + " kernel32.lib /out:" + props.outputFile + "";
            LOG_TRACE("Running command: {}", linkCommand);
            if (std::system(linkCommand.c_str()) != 0)
            {
                LOG_ERROR("Failed to execute command: {}", linkCommand);
                return 1;
            }
            std::string runCommand = "\"" + std::string(props.outputFile) + "\"";
            LOG_INFO("Running command: {}", runCommand);
            int exitCode = std::system(runCommand.c_str());
            LOG_INFO("Program exited with code: {}", exitCode);
            break;
        }
        }

        return 0;
    }

    std::vector<Token> Compiler::tokenize(const std::string &source)
    {
        std::vector<Token> tokens;
        for (int i = 0; i < source.size(); i++)
        {
            char c = source[i];
            if (isspace(c))
                continue;
            else if (isalpha(c))
            {
                std::string ident;
                while (i < source.size() && isalnum(source[i]))
                    ident += source[i++];
                i--;
                if (ident == "return")
                    tokens.push_back({TokenType::_return, std::nullopt});
                else
                    LOG_ERROR("Unknown identifier: {}", ident);
            }
            else if (isdigit(c))
            {
                std::string number;
                while (i < source.size() && isdigit(source[i]))
                    number += source[i++];
                i--;
                tokens.push_back({TokenType::int_literal, number});
            }
            else if (c == ';')
            {
                tokens.push_back({TokenType::semicolon, std::nullopt});
            }
            else
            {
                LOG_ERROR("Unknown character: {}", c);
            }
        }
        return tokens;
    }

    std::string Compiler::assemble(const std::vector<Token> &tokens)
    {
        std::string output;
        output += "global _start\n";      // ASM: Text Section
        output += "extern ExitProcess\n"; // ASM: WinAPI ExitProcess
        output += "\n";                   // ASM:
        output += "section .text\n";      // ASM: Entry Point definition
        output += "_start:\n";            // ASM: Start Label
        for (int i = 0; i < tokens.size(); i++)
        {
            const Token &token = tokens[i];
            if (token.type == TokenType::_return)
            {
                if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::int_literal)
                {
                    if (i + 2 < tokens.size() && tokens[i + 2].type == TokenType::semicolon)
                    {
                        output += "\tsub rsp, 40\n";                                  // ASM: Align Stack
                        output += "\tmov ecx, " + tokens[i + 1].value.value() + "\n"; // ASM: Move literal to ecx
                        output += "\tcall ExitProcess\n";                             // ASM: Exit
                        i += 2;                                                       // Skip the next two tokens
                    }
                    else
                    {
                        LOG_ERROR("Expected ';' after integer literal");
                    }
                }
                else
                {
                    LOG_ERROR("Expected integer literal after 'return'");
                }
            }
        }
        return output;
    }
}