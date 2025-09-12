#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "Error.h"

namespace Delta
{
    class Log
    {
    public:
        static void init(std::string logFile = "log.txt");
        static void setVerbose(bool verbose);

    public:
        static std::shared_ptr<spdlog::logger> s_Logger;
    };
}

#define LOG_INFO(...) ::Delta::Log::s_Logger->info(__VA_ARGS__)
#define LOG_WARN(...) ::Delta::Log::s_Logger->warn(__VA_ARGS__)
#define LOG_ERROR(...)                          \
    ::Delta::Log::s_Logger->error(__VA_ARGS__); \
    BREAKPOINT()
#define LOG_DEBUG(...) ::Delta::Log::s_Logger->debug(__VA_ARGS__)
#define LOG_TRACE(...) ::Delta::Log::s_Logger->trace(__VA_ARGS__)