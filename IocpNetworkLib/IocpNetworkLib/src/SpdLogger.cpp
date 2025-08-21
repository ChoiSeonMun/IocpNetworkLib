#include "pch.h"
#include "csmnet/util/SpdLogger.h"

using namespace std;

namespace csmnet::util
{
    SpdConsoleLogger::SpdConsoleLogger(const std::string& pattern) noexcept
    {
        spdlog::set_pattern(pattern, spdlog::pattern_time_type::utc);
    }
    void SpdConsoleLogger::Info(std::string_view log)
    {
        spdlog::info(log);
    }

    void SpdConsoleLogger::Warn(std::string_view log)
    {
        spdlog::warn(log);
    }

    void SpdConsoleLogger::Error(std::string_view log)
    {
        spdlog::error(log);
    }
}


