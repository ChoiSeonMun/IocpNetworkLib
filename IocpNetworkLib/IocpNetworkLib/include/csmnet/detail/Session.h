#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/Socket.h"
#include "csmnet/Endpoint.h"

namespace csmnet
{
    class Session
    {
    public:
        Session() noexcept = default;
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;
        Session(Session&& other) noexcept
            : _isConnected(other._isConnected.load(std::memory_order_acquire))
            , _socket(std::move(other._socket))
            , _remote(std::move(other._remote))
        {
            other._isConnected.store(false, std::memory_order_release);
        }

        Session& operator=(Session&& other) noexcept
        {
            if (this != &other)
            {
                _isConnected.store(other._isConnected.load(std::memory_order_acquire), std::memory_order_release);
                _socket = std::move(other._socket);
                _remote = std::move(other._remote);
                other._isConnected.store(false, std::memory_order_release);
            }
            return *this;
        }

        virtual ~Session() = default;

        bool IsConnected() const noexcept { return _isConnected; }
        const Endpoint& GetRemoteEndpoint() const noexcept { return _remote; }

        virtual expected<void, error_code> Send(std::span<const std::byte> data) noexcept  = 0;
        virtual void Disconnect() noexcept = 0;

        virtual void OnConnected() {}
        virtual void OnDisconnected() {}
    protected:
        std::atomic<bool> _isConnected{ false };
        detail::Socket _socket;
        Endpoint _remote = Endpoint::Any(0);
    };
}
