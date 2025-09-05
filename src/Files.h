#pragma once

#include <string>
#include <filesystem>

namespace Delta
{
    class Files
    {
    public:
        static bool fileExists(const std::string &path);
        static std::string readFile(const std::string &path);
        static bool writeFile(const std::string &path, const std::string &content);
        static std::string getFileExtension(const std::string &filename);
        static std::string getFileName(const std::string &path);
        static std::string getDirectory(const std::string &path);
        static bool createDirectory(const std::string &path);
        static std::string getProgramPath();
        static std::string joinPaths(const std::string &path1, const std::string &path2);
    };
}