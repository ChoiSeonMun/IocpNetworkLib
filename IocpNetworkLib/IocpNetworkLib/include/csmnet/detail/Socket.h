#pragma once

#include "Config.h"
#include "SocketOption.h"
#include "IIocpRegistrable.h"
#include "csmnet/Endpoint.h"

#include <span>

namespace csmnet::detail
{
    using namespace std;

    enum class SocketType
    {
        Tcp,
        Udp
    };
    
    class SendEvent;
    class RecvEvent;
    class AcceptEvent;
    class ConnectEvent;
    class DisconnectEvent;
    enum class SocketState
    {
        Closed,
        Opened,
        Bound,
        Listening,
        Connected
    };

    class Socket final : public IIocpRegistrable
    {
    public:
        using Linger = Linger<SOL_SOCKET, SO_LINGER>;
        using ReuseAddress = Boolean<SOL_SOCKET, SO_REUSEADDR>;
        using UpdateAcceptContext = UpdateAcceptContext<SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT>;

        Socket() noexcept = default;
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;
        ~Socket() noexcept
        {
            Close();
        }

        bool IsOpen() const noexcept { return _state != SocketState::Closed; }
        expected<void, error_code> Open(SocketType type) noexcept;
        void Close() noexcept;

        expected<void, error_code> Bind(const Endpoint& local) noexcept;
        expected<void, error_code> Listen(int32 backlog = SOMAXCONN) noexcept;
        expected<void, error_code> AcceptEx(AcceptEvent& event) noexcept;
        expected<void, error_code> ConnectEx(ConnectEvent& event) noexcept;
        expected<void, error_code> DisconnectEx(DisconnectEvent& event) noexcept;
        expected<void, error_code> SendEx(SendEvent& event) noexcept;
        expected<void, error_code> RecvEx(RecvEvent& event) noexcept;

        template <typename SocketOption>
        void SetOption(const SocketOption& option) noexcept
        {
            ::setsockopt(_socket, option.GetLevel(), option.GetName(), reinterpret_cast<const char*>(option.GetData()), option.GetSize());
        }

        operator SOCKET() const noexcept { return _socket; }
        
        HANDLE GetHandle() const noexcept override { return reinterpret_cast<HANDLE>(_socket); }
    private:
        SOCKET _socket = INVALID_SOCKET;
        SocketState _state = SocketState::Closed;
    };
}