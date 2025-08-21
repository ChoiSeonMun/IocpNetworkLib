#pragma once

#include "ILogger.h"

#include <expected>
#include <spdlog/spdlog.h>

namespace csmnet::util
{
    class SpdConsoleLogger : public ILogger
    {
    public:
        SpdConsoleLogger(const std::string& pattern = "[%Y-%m-%dT%H:%M:%S] [%l] %v") noexcept;
        
        void Info(std::string_view log) override;
        void Warn(std::string_view log) override;
        void Error(std::string_view log) override;
    };
}