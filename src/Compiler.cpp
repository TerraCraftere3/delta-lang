#include "Compiler.h"
#include "Log.h"
#include "Files.h"
#include "Tokenizer.h"
#include "Preprocessor.h"
#include "Parser.h"
#include "Assembler.h"
#include "Error.h"
#include "Debug.h"
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Delta
{
    int runProgram(const std::string &programPath)
    {
#ifdef _WIN32
        STARTUPINFO si = {sizeof(si)};
        PROCESS_INFORMATION pi;

        BOOL success = CreateProcess(
            nullptr,
            const_cast<char *>(programPath.c_str()), // command line
            nullptr, nullptr, FALSE,
            CREATE_NEW_CONSOLE, nullptr, nullptr,
            &si, &pi);

        if (!success)
            return -1;

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return static_cast<int>(exitCode);
#endif
    }

    bool Compiler::generateWasmHtml(const std::string &htmlPath, const std::string &wasmFile)
    {
        std::string html = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>WebAssembly Application</title>
    <style>
        body { font-family: monospace; padding: 20px; }
        #output { 
            background: #1e1e1e; 
            color: #d4d4d4; 
            padding: 15px; 
            border-radius: 5px;
            white-space: pre-wrap;
            min-height: 200px;
        }
    </style>
</head>
<body>
    <h1>WebAssembly Output</h1>
    <div id="output"></div>
    <script>
        const output = document.getElementById('output');
        let memory;
        
        function writeOutput(text) {
            output.textContent += text;
        }
        
        // helper: read c string from memory
        function readCString(ptr) {
            if (!memory) return '';
            const view = new Uint8Array(memory.buffer);
            let str = '';
            let i = ptr;
            while (view[i] !== 0) {
                str += String.fromCharCode(view[i]);
                i++;
            }
            return str;
        }

        // helper: read 32-bit int from memory
        function readInt32(ptr) {
            const view = new DataView(memory.buffer);
            return view.getInt32(ptr, true); // little endian
        }

        // helper: read double from memory
        function readDouble(ptr) {
            const view = new DataView(memory.buffer);
            return view.getFloat64(ptr, true); // little endian
        }

        function readFormattedString(formatPtr, argsPtr) {
            const format = readCString(formatPtr);
            let result = '';

            let argOffset = 0;
            for (let i = 0; i < format.length; i++) {
                if (format[i] === '%' && i + 1 < format.length) {
                    i++;
                    // Handle 'll' prefix for long long
                    let spec = format[i];
                    if (spec === 'l' && i + 2 < format.length && format[i + 1] === 'l') {
                        spec = 'll';
                        i += 2; // skip both l's
                    }

                    switch (spec) {
                        case 'd': { // int
                            const value = readInt32(argsPtr + argOffset);
                            result += value;
                            argOffset += 4;
                            break;
                        }
                        case 'f': { // double
                            const value = readDouble(argsPtr + argOffset);
                            result += value;
                            argOffset += 8;
                            break;
                        }
                        case 's': { // string
                            const strPtr = readInt32(argsPtr + argOffset);
                            result += readCString(strPtr);
                            argOffset += 4;
                            break;
                        }
                        case 'c': { // char
                            const value = readInt32(argsPtr + argOffset);
                            result += String.fromCharCode(value);
                            argOffset += 4;
                            break;
                        }
                        case '%': { // literal %
                            result += '%';
                            break;
                        }
                        default: {
                            // unknown specifier, just output as-is
                            result += '%' + spec;
                            break;
                        }
                    }
                } else {
                    result += format[i];
                }
            }

            return result;
        }

        
        
        // Import object with runtime functions
        const importObject = {
            env: {
                // Standard C library functions
                printf: (formatPtr, argsPtr) => {
                    const text = readFormattedString(formatPtr, argsPtr);
                    writeOutput(text);
                    return text.length;
                },
                puts: (strPtr) => {
                    const text = readCString(strPtr);
                    writeOutput(text + '\n');
                    return text.length;
                },
                
                putchar: (char) => {
                    writeOutput(String.fromCharCode(char));
                    return char;
                },
                
                // Memory allocation (simple implementation)
                malloc: (size) => {
                    // This is a placeholder - proper implementation needed
                    writeOutput(`malloc(${size}) called\n`);
                    return 0;
                },
                
                free: (ptr) => {
                    // Placeholder
                },
                
                // Math functions
                sin: Math.sin,
                cos: Math.cos,
                tan: Math.tan,
                sqrt: Math.sqrt,
                pow: Math.pow,
                floor: Math.floor,
                ceil: Math.ceil,
                
                // Exit
                exit: (code) => {
                    writeOutput(`\nProgram exited with code: ${code}\n`);
                },
                
                // Abort
                abort: () => {
                    writeOutput('\nProgram aborted\n');
                }
            },
            
            wasi_snapshot_preview1: {
                // WASI functions if needed
                fd_write: (fd, iovs, iovsLen, nwritten) => {
                    // Simplified fd_write for stdout
                    return 0;
                },
                
                proc_exit: (code) => {
                    writeOutput(`\nProcess exited with code: ${code}\n`);
                }
            }
        };
        
        // Load and instantiate WASM
        writeOutput('Loading WebAssembly module...\n');
        
        fetch(')" + wasmFile +
                           R"(')
            .then(response => response.arrayBuffer())
            .then(bytes => WebAssembly.instantiate(bytes, importObject))
            .then(result => {
                memory = result.instance.exports.memory;
                
                writeOutput('Module loaded successfully!\n');
                writeOutput('---\n');
                
                // Call main or exported functions
                if (result.instance.exports.main) {
                    try {
                        const exitCode = result.instance.exports.main();
                        writeOutput('\n---\n');
                        writeOutput(`Program completed with exit code: ${exitCode || 0}\n`);
                    } catch (e) {
                        writeOutput('\n---\n');
                        writeOutput(`Runtime error: ${e.message}\n`);
                        console.error(e);
                    }
                } else if (result.instance.exports._start) {
                    try {
                        result.instance.exports._start();
                        writeOutput('\n---\n');
                        writeOutput('Program completed\n');
                    } catch (e) {
                        writeOutput('\n---\n');
                        writeOutput(`Runtime error: ${e.message}\n`);
                        console.error(e);
                    }
                } else {
                    writeOutput('No main() or _start() function found in WASM module\n');
                }
            })
            .catch(err => {
                writeOutput(`Error: ${err.message}\n`);
                console.error('WASM Error:', err);
            });
    </script>
</body>
</html>)";

        if (!Files::writeFile(htmlPath, html))
        {
            LOG_ERROR("Failed to write HTML file: {}", htmlPath);
            return false;
        }

        LOG_INFO("Generated HTML file: {}", htmlPath);
        return true;
    }

    bool Compiler::openInBrowser(const std::string &htmlPath)
    {
        LOG_INFO("Starting local HTTP server for: {}", htmlPath);

        std::string directory = Files::getDirectory(htmlPath);
        std::string htmlFile = Files::getFileName(htmlPath);

#ifdef _WIN32
        // Use Python's built-in HTTP server
        std::string serverCommand = "start cmd /K \"cd /d \"" + directory + "\" && python -m http.server 8000\"";
        std::system(serverCommand.c_str());

        // Wait a moment for server to start
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        // Open browser to localhost
        std::string browserCommand = "start \"\" \"http://localhost:8000/" + htmlFile + "\"";
        return std::system(browserCommand.c_str()) == 0;

#elif __APPLE__
        std::string serverCommand = "cd \"" + directory + "\" && python3 -m http.server 8000 &";
        std::system(serverCommand.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        std::string browserCommand = "open \"http://localhost:8000/" + htmlFile + "\"";
        return std::system(browserCommand.c_str()) == 0;

#else
        std::string serverCommand = "cd \"" + directory + "\" && python3 -m http.server 8000 &";
        std::system(serverCommand.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        std::string browserCommand = "xdg-open \"http://localhost:8000/" + htmlFile + "\"";
        return std::system(browserCommand.c_str()) == 0;
#endif
    }

    int Compiler::compile(const CompilerProperties &props)
    {
        Log::setVerbose(props.verbose);
        std::string inputPath = Files::getAbsolutePath(props.inputFile);
        std::string outputPath = Files::getAbsolutePath(props.outputFile);
        LOG_INFO("Compiling {}...", inputPath);
        LOG_TRACE("Compile Type: {}", getCompileTypeName(props.compileType));

        bool isWasm = (props.compileTarget == TARGET_WASM);

        if (!Files::fileExists(inputPath))
        {
            LOG_ERROR("Input file does not exist: {}", inputPath);
            return 1;
        }

        std::string programPath = Files::getDirectory(Files::getProgramPath());
        std::string llcLink = Files::joinPaths(programPath, "llc.exe");
        if (!Files::fileExists(llcLink))
        {
            LOG_ERROR("LLVM Compiler executable not found at: {}", llcLink);
            return 1;
        }

        std::string linkPath = isWasm ? Files::joinPaths(programPath, "wasm-ld.exe") : Files::joinPaths(programPath, "clang.exe");
        if (!Files::fileExists(linkPath))
        {
            LOG_ERROR("Linker executable not found at: {}", linkPath);
            return 1;
        }

        std::string stdlibPath = Files::joinPaths(programPath, "stdlib.lib");
        if (!Files::fileExists(stdlibPath))
        {
            LOG_ERROR("Standard Library not found at: {}", stdlibPath);
            return 1;
        }

        std::string contents = Files::readFile(inputPath);
        if (contents.empty())
        {
            LOG_ERROR("Failed to read input file or file is empty: {}", inputPath);
            return 1;
        }

        std::string assembly = "";

        std::string intDir = Files::joinPaths(Files::getDirectory(outputPath), "int");
        if (!Files::fileExists(intDir))
        {
            if (!Files::createDirectory(intDir))
            {
                LOG_ERROR("Failed to create intermediate directory: {}", intDir);
                return 1;
            }
        }
        auto writeParseFile = [&](const std::string &parseFile, const std::string &content) -> bool
        {
            if (!Files::writeFile(parseFile, content))
            {
                LOG_ERROR("Failed to write intermediate file: {}", parseFile);
                return false;
            }
            return true;
        };
        auto writeAsmFile = [&](const std::string &asmFile) -> bool
        {
            if (!Files::writeFile(asmFile, assembly))
            {
                LOG_ERROR("Failed to write intermediate file: {}", asmFile);
                return false;
            }
            return true;
        };

        auto runCommand = [&](const std::string &cmd) -> bool
        {
            LOG_TRACE("Running command: cmd /C {}", cmd);
            if (std::system(("cmd /C " + cmd).c_str()) != 0)
            {
                LOG_ERROR("Failed to execute command");
                return false;
            }
            return true;
        };

        // Generate file paths
        std::string baseName = Files::getFileNameWithoutExtension(inputPath);
        std::string parseFile = Files::joinPaths(intDir, baseName + ".ast.txt");
        std::string asmFile = Files::joinPaths(intDir, baseName + ".ll");
        std::string objExtension = isWasm ? ".wasm.o" : ".obj";
        std::string outputExtension = isWasm ? ".wasm" : ".exe";
        std::string objFile = Files::joinPaths(intDir, baseName + objExtension);

        std::string stdlibIncludePath = Files::joinPaths(Files::getDirectory(Files::getDirectory(Files::getProgramPath())), "stdlib");
        std::vector<std::string> includeDirs = {Files::getDirectory(inputPath), stdlibIncludePath};
        includeDirs.insert(includeDirs.end(), props.includeDirs.begin(), props.includeDirs.end());

        std::string llcArguments = "";
        if (isWasm)
            llcArguments = "-march=wasm32 -filetype=obj";
        else
            llcArguments = "-filetype=obj";
        std::string linkArguments = "";
        if (isWasm)
        {
            linkArguments = "--no-entry --export-dynamic --allow-undefined";
        }
        else
        {
#ifdef _WIN32
            for (auto link : props.additionalLinks)
            {
                linkArguments += " " + Files::getAbsolutePath(link);
            }
            linkArguments += " -luser32 -lgdi32 -lkernel32 -lopengl32";
#endif
        }

#ifdef _WIN32
        std::string linkOutputFlag = " -o " + std::string(outputPath);
#endif

        std::string assemblyPrefix = "; Generated by Delta Compiler\n";
        assemblyPrefix += "; Input File: " + std::string(inputPath) + "\n";
        assemblyPrefix += "; Compiler Arguments: " + llcArguments + "\n";
        assemblyPrefix += "; Linker Arguments: " + linkArguments + "\n";

        Tokenizer tokenizer(contents);
        std::vector<Token> tokens = tokenizer.tokenize();
        Preprocessor processor(tokens);
        std::vector<Token> processedTokens = processor.process(includeDirs).tokens;
        Parser parser(processedTokens);
        auto parseTree = parser.parseProgram();
        if (parseTree.has_value())
        {
            writeParseFile(parseFile, nodeDebugPrint(parseTree.value()));
            Assembler assembler(parseTree.value());
            assembly = assembler.generate();
            assembly = assemblyPrefix + assembly;
        }
        else
        {
            LOG_ERROR("Invalid Program");
            return 1;
        }

        switch (props.compileType)
        {
        case COMPILE_ONLY:
        {
            if (!writeAsmFile(asmFile))
                return 1;
            std::string command = "\"" + llcLink + "\" " + llcArguments + " " + asmFile + " -o " + outputPath;
            if (!runCommand(command))
                return 1;
            break;
        }
        case COMPILE_AND_LINK:
        {
            if (!writeAsmFile(asmFile))
                return 1;
            std::string assembleCommand = "\"" + llcLink + "\" " + llcArguments + " " + asmFile + " -o " + objFile;
            if (!runCommand(assembleCommand))
                return 1;

            std::string linkCommand = "\"" + linkPath + "\" " + objFile;
            if (!isWasm)
            {
                linkCommand += " " + stdlibPath + linkOutputFlag;
            }
            else
            {
                linkCommand += " -o " + outputPath;
            }
            linkCommand += " " + linkArguments;

            if (!runCommand(linkCommand))
                return 1;
            break;
        }

        case COMPILE_LINK_AND_RUN:
        {
            if (!writeAsmFile(asmFile))
                return 1;
            std::string assembleCommand = "\"" + llcLink + "\" " + llcArguments + " " + asmFile + " -o " + objFile;
            if (!runCommand(assembleCommand))
                return 1;

            std::string linkCommand = "\"" + linkPath + "\" " + objFile;
            if (!isWasm)
            {
                linkCommand += " " + stdlibPath + linkOutputFlag;
            }
            else
            {
                linkCommand += " -o " + outputPath;
            }
            linkCommand += " " + linkArguments;

            if (!runCommand(linkCommand))
                return 1;

            if (isWasm)
            {
                // Generate HTML file and run in browser
                std::string htmlPath = Files::replaceExtension(outputPath, ".html");
                if (!generateWasmHtml(htmlPath, Files::getFileName(outputPath)))
                    return 1;

                if (!openInBrowser(htmlPath))
                    return 1;
            }
            else
            {
                std::string runCommandStr = "\"" + std::string(outputPath) + "\"";
                LOG_INFO("Running command: {}", runCommandStr);
                int exitCode = runProgram(std::string(outputPath));
                LOG_INFO("Program exited with code: {}", exitCode);
            }
            break;
        }
        }

        return 0;
    }
}