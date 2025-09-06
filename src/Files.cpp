#include "Files.h"

#include <fstream>
#include "Log.h"

#if defined(_WIN32)
#include <windows.h>
std::string GetExecutablePath()
{
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0)
        return {};
    return std::string(buffer, length);
}

#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
std::string GetExecutablePath()
{
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length == -1)
        return {};
    buffer[length] = '\0';
    return std::string(buffer);
}

#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
std::string GetExecutablePath()
{
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) != 0)
        return {}; // buffer too small
    return std::string(buffer);
}

#else
#error "Unsupported platform"
#endif

bool Delta::Files::fileExists(const std::string &path) { return std::filesystem::exists(path); }

std::string Delta::Files::readFile(const std::string &path)
{
    if (!fileExists(path))
        return std::string();

    std::ifstream file(path);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

bool Delta::Files::writeFile(const std::string &path, const std::string &content)
{
    std::ofstream file(path);
    if (!file)
    {
        LOG_ERROR("Failed to open file for writing: {}", path);
        return false;
    }
    file << content;
    return true;
}

std::string Delta::Files::getFileExtension(const std::string &filename)
{
    std::filesystem::path path(filename);
    return path.extension().string();
}

std::string Delta::Files::getFileName(const std::string &path)
{
    std::filesystem::path p(path);
    return p.filename().string();
}

std::string Delta::Files::getFileNameWithoutExtension(const std::string &path)
{
    std::filesystem::path p(path);
    return p.stem().string();
}

std::string Delta::Files::getDirectory(const std::string &path)
{
    std::filesystem::path p(path);
    return p.parent_path().string();
}

bool Delta::Files::createDirectory(const std::string &path)
{
    std::filesystem::path p(path);
    return std::filesystem::create_directory(p);
}

std::string Delta::Files::getProgramPath() { return GetExecutablePath(); }

std::string Delta::Files::getWorkingDirectory() { return std::filesystem::current_path().string(); }

std::string Delta::Files::joinPaths(const std::string &path1,
                                    const std::string &path2)
{
    return (std::filesystem::path(path1) / path2).string();
}
