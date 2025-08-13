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
        static optional<Endpoint> From(std::string_view ip, uint16 port) noexcept;

        std::string GetIp() const;
        uint16 GetPort() const noexcept { return ::ntohs(_addr.sin_port); }
        sockaddr* GetNative() noexcept { return reinterpret_cast<sockaddr*>(&_addr); }
        const sockaddr* GetNative() const noexcept { return reinterpret_cast<const sockaddr*>(&_addr); }
        size_t GetSize() const noexcept { return sizeof(_addr); }
        socklen_t GetCapacity() const noexcept { return sizeof(_addr); }
    private:
        Endpoint(sockaddr_in sockaddr) noexcept : _addr(sockaddr) { }
    private:
        sockaddr_in _addr;
    };
}