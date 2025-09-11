#pragma once

#include "spdlog/spdlog.h"
#include "Error.h"

namespace Delta
{
    class Log
    {
    public:
        static void init();
        static void setVerbose(bool verbose);
    };
}

#define LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define LOG_WARN(...) spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...)          \
    spdlog::error(__VA_ARGS__); \
    BREAKPOINT()
#define LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define LOG_TRACE(...) spdlog::trace(__VA_ARGS__)