#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/detail/Socket.h"
#include "csmnet/detail/IocpEvent.h"

#include <thread>

namespace csmnet
{
    class Endpoint;
    class Client : public detail::IIocpEventProcessor
    {
    public:
        Client() noexcept = default;
        virtual ~Client() noexcept = default;
        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;
        Client(Client&&) noexcept = default;
        Client& operator=(Client&&) noexcept = default;

        expected<void, error_code> Connect(const Endpoint& serverEndpoint) noexcept;
        expected<void, error_code> Disconnect() noexcept;
    private:
        void Process(detail::ConnectEvent* event) override;
        void Process(detail::DisconnectEvent* event) override;
    private:
        void ProcessIO();
    private:
        std::atomic<bool> _isConnected{ false };
        detail::IocpCore _iocpCore;
        detail::Socket _socket;
        detail::ConnectEvent _connectEvent{ this };
        detail::DisconnectEvent _disconnectEvent{ this };
        std::jthread _ioThread;
    };
}
