#include "Log.h"

namespace Delta
{
    std::shared_ptr<spdlog::logger> Log::s_Logger = nullptr;

    void Log::init(std::string logFile)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%T] [%^%l%$] %v"); // [12:34:56] [INFO] message

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
        file_sink->set_pattern("[%Y-%m-%d %T] [%l] %v");

        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        s_Logger = std::make_shared<spdlog::logger>("DeltaLogger", sinks.begin(), sinks.end());

        s_Logger->set_level(spdlog::level::info);
        s_Logger->flush_on(spdlog::level::trace);
        spdlog::register_logger(s_Logger);
    }

    void Log::setVerbose(bool verbose)
    {
        if (verbose)
            s_Logger->set_level(spdlog::level::trace); // more detail
        else
            s_Logger->set_level(spdlog::level::info); // default
    }
}
