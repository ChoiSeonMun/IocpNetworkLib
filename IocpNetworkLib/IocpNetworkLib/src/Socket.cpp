#include "pch.h"
#include "csmnet/detail/Socket.h"
#include "csmnet/Endpoint.h"
#include "csmnet/detail/IocpEvent.h"

using namespace std;

namespace csmnet::detail
{
    expected<void, error_code> Socket::Open(SocketType type) noexcept
    {
        if (IsOpen())
        {
            return unexpected(LibError::SocketAlreadyOpen);
        }

        switch (type)
        {
        case SocketType::Tcp:
            _socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
            break;
        case SocketType::Udp:
            _socket = ::WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
            break;
        default:
            break;
        }

        if (_socket == INVALID_SOCKET)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        _state = SocketState::Opened;
        return {};
    }

    void Socket::Close() noexcept
    {
        if (IsOpen())
        {
            ::closesocket(_socket);
            _socket = INVALID_SOCKET;
            _state = SocketState::Closed;
        }
    }

    Socket::Socket(Socket&& other) noexcept
        :_socket(exchange(other._socket, INVALID_SOCKET)),
        _state(exchange(other._state, SocketState::Closed))
    {
    }

    Socket& Socket::operator=(Socket&& other) noexcept
    {
        if (this != &other)
        {
            Close();
            _socket = exchange(other._socket, INVALID_SOCKET);
            _state = exchange(other._state, SocketState::Closed);
        }

        return *this;
    }

    expected<void, error_code> Socket::Bind(const Endpoint& local) noexcept
    {
        if (IsOpen() == false)
        {
            return unexpected(LibError::SocketNotOpen);
        }

        int32 result = ::bind(_socket, local.GetNative(), local.GetSize());
        if (result == SOCKET_ERROR)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        _state = SocketState::Bound;
        return {};
    }

    expected<void, error_code> Socket::Listen(int32 backlog) noexcept
    {
        if (IsOpen() == false)
        {
            return unexpected(LibError::SocketNotOpen);
        }

        if (_state != SocketState::Bound)
        {
            return unexpected(LibError::SocketNotBound);
        }

        int32 result = ::listen(_socket, backlog);
        if (result == SOCKET_ERROR)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        _state = SocketState::Listening;
        return {};
    }

    expected<void, error_code> Socket::AcceptEx(AcceptEvent& event) noexcept
    {
        if (IsOpen() == false)
        {
            return unexpected(LibError::SocketNotOpen);
        }

        if (_state != SocketState::Listening)
        {
            return unexpected(LibError::SocketNotListening);
        }

        BOOL result = ::AcceptEx(_socket,
            event.GetAcceptSocket(),
            event.GetBuffer(),
            0,
            sizeof(sockaddr_in) + 16,
            sizeof(sockaddr_in) + 16,
            reinterpret_cast<LPDWORD>(event.GetBytesTransferredData()),
            event.GetOverlapped());

        if (result == FALSE && ::WSAGetLastError() != WSA_IO_PENDING)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::ConnectEx(const Endpoint& remote, ConnectEvent& event) noexcept
    {
        if (IsOpen() == false)
        {
            return unexpected(LibError::SocketNotOpen);
        }

        if (_state != SocketState::Bound)
        {
            return unexpected(LibError::SocketNotBound);
        }

        BOOL result = WinsockExtension::ConnectEx(
            _socket,
            remote.GetNative(),
            remote.GetSize(),
            nullptr,
            0,
            reinterpret_cast<LPDWORD>(event.GetBytesTransferredData()),
            event.GetOverlapped()
        );

        if (result == FALSE && ::WSAGetLastError() != WSA_IO_PENDING)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        _state = SocketState::Connected;
        return {};
    }

    expected<void, error_code> Socket::DisconnectEx(DisconnectEvent& event) noexcept
    {
        if (_state != SocketState::Connected)
        {
            return unexpected(LibError::SocketNotConnected);
        }

        BOOL result = WinsockExtension::DisconnectEx(
            _socket,
            event.GetOverlapped(),
            TF_REUSE_SOCKET,
            0
        );

        if (!result)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        _state = SocketState::Closed;
        return {};
    }
}
