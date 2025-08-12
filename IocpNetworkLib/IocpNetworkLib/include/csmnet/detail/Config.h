#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <utility>
#include <expected>
#include <optional>

#include "csmnet/common/Error.h"

namespace csmnet
{
    template<typename T, typename E>
    using expected = std::expected<T, E>;
    
    using error_code = std::error_code;
    
    template <typename T>
    using optional = std::optional;
}

#pragma comment(lib, "Ws2_32.lib")
