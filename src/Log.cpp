#include "Log.h"

void Delta::Log::init()
{
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::info);
}

void Delta::Log::setVerbose(bool verbose)
{
    spdlog::set_level(verbose ? spdlog::level::trace : spdlog::level::info);
}
