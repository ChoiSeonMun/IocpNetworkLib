#pragma once

#include <string_view>

namespace csmnet::util
{
    class ILogger
    {
    public:
        virtual ~ILogger() noexcept = default;
        virtual void Info(std::string_view log) = 0;
        virtual void Warn(std::string_view log) = 0;
        virtual void Error(std::string_view log) = 0;
    };
}