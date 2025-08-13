#include "pch.h"
#include "csmnet/Endpoint.h"

#include <array>

using namespace std;

namespace csmnet
{
    Endpoint Endpoint::Any(uint16 port) noexcept
    {
        sockaddr_in addr = { };
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        return Endpoint(addr);
    }

    optional<Endpoint> Endpoint::From(std::string_view ip, uint16 port) noexcept
    {
        sockaddr_in addr{ };
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.data(), &addr.sin_addr) == 0)
        {
            return nullopt;
        }

        return Endpoint(addr);
    }

    std::string Endpoint::GetIp() const
    {
        array<char, INET_ADDRSTRLEN> buffer{ };
        if (inet_ntop(AF_INET, &_addr.sin_addr, buffer.data(), buffer.size()))
        {
            return string(buffer.data());
        }

        return "";
    }
}