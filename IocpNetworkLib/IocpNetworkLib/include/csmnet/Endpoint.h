#pragma once

#include "csmnet/detail/Config.h"

#include <string_view>
#include <string>

namespace csmnet
{
    class Endpoint final
    {
    public:
        static Endpoint Any(uint16 port) noexcept;
        static expected<Endpoint, error_code> From(std::string_view ip, uint16 port) noexcept;

        explicit Endpoint(sockaddr_in sockaddr) noexcept : _addr(sockaddr) {}

        std::string GetIp() const;
        uint16 GetPort() const noexcept { return ::ntohs(_addr.sin_port); }
        sockaddr* GetNative() noexcept { return reinterpret_cast<sockaddr*>(&_addr); }
        const sockaddr* GetNative() const noexcept { return reinterpret_cast<const sockaddr*>(&_addr); }
        size_t GetSize() const noexcept { return sizeof(_addr); }
        socklen_t GetCapacity() const noexcept { return sizeof(_addr); }
    private:
        sockaddr_in _addr;
    };
}