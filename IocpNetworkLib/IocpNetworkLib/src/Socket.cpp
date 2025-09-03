#include "pch.h"
#include "csmnet/detail/Socket.h"
#include "csmnet/Endpoint.h"
#include "csmnet/detail/IocpEvent.h"

using namespace std;

namespace csmnet::detail
{
    expected<void, error_code> Socket::Open() noexcept
    {
        if (IsOpen())
        {
            return unexpected(LibError::SocketAlreadyOpen);
        }

        _socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);        
        if (_socket == INVALID_SOCKET)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    void Socket::Close() noexcept
    {
        if (IsOpen())
        {
            ::closesocket(_socket);
            _socket = INVALID_SOCKET;
        }
    }

    Socket::Socket(Socket&& other) noexcept
        :_socket(exchange(other._socket, INVALID_SOCKET))
    {
    }

    Socket& Socket::operator=(Socket&& other) noexcept
    {
        if (this != &other)
        {
            Close();
            _socket = exchange(other._socket, INVALID_SOCKET);
        }

        return *this;
    }

    expected<void, error_code> Socket::Shutdown(ShutdownKind kind) noexcept
    {
        int32 result = ::shutdown(_socket, static_cast<int>(kind));
        if (result == SOCKET_ERROR)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::Bind(const Endpoint& local) noexcept
    {
        int32 result = ::bind(_socket, local.GetNative(), local.GetSize());
        if (result == SOCKET_ERROR)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::Listen(int32 backlog) noexcept
    {
        int32 result = ::listen(_socket, backlog);
        if (result == SOCKET_ERROR)
        {
            // 나올 수 있는 오류 코드: WSAENOTSOCK, WSAEISCONN, WSAEINVAL
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::AcceptEx(AcceptEvent& event) noexcept
    {
        BOOL result = ::AcceptEx(_socket,
            event.GetAcceptSocket(),
            event.GetBuffer(),
            0,
            sizeof(sockaddr_in) + 16,
            sizeof(sockaddr_in) + 16,
            reinterpret_cast<LPDWORD>(event.GetBytesTransferredData()),
            event.GetOverlappedData());

        if (result == FALSE && ::WSAGetLastError() != WSA_IO_PENDING)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::ConnectEx(ConnectEvent& event) noexcept
    {
        BOOL result = WinsockExtension::ConnectEx(
            _socket,
            event.GetRemote().GetNative(),
            event.GetRemote().GetSize(),
            nullptr,
            0,
            nullptr,
            event.GetOverlappedData()
        );

        if (result == FALSE && ::WSAGetLastError() != WSA_IO_PENDING)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::DisconnectEx(DisconnectEvent& event) noexcept
    {
        BOOL result = WinsockExtension::DisconnectEx(
            _socket,
            nullptr,
            TF_REUSE_SOCKET,
            0
        );

        if (!result && WSAGetLastError() != WSA_IO_PENDING)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::SendEx(SendEvent& event) noexcept
    {
        auto result = WSASend(_socket,
            event.GetData(),
            event.GetBufferCount(),
            nullptr,
            0,
            event.GetOverlappedData(),
            nullptr);

        if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::RecvEx(RecvEvent& event) noexcept
    {
        if (IsOpen() == false)
        {
            return unexpected(LibError::InvalidSocket);
        }

        auto result = WSARecv(_socket,
            event.GetData(),
            event.GetBufferCount(),
            nullptr,
            reinterpret_cast<LPDWORD>(event.GetFlagsData()),
            event.GetOverlappedData(),
            nullptr);

        if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return {};
    }

    expected<void, error_code> Socket::CancelEvent(IocpEvent* event) noexcept
    {
        CSM_ASSERT(IsOpen());

        auto result = ::CancelIoEx(GetHandle(), event ? event->GetOverlappedData() : nullptr);
        
        if (result == FALSE)
        {
            if (int32 error = GetLastError(); error != ERROR_NOT_FOUND)
            {
                return unexpected(error_code(error, std::system_category()));
            }
        }

        return {};
    }

    expected<Endpoint, error_code> Socket::GetRemoteEndpoint() const noexcept
    {
        CSM_ASSERT(IsOpen());

        sockaddr_in addr{ };
        socklen_t addrLen = sizeof(sockaddr_in);
        int result = ::getpeername(_socket,
            reinterpret_cast<sockaddr*>(&addr),
            &addrLen);

        if (result == SOCKET_ERROR)
        {
            return unexpected(TranslateWsaError(WSAGetLastError()));
        }

        return Endpoint(addr);
    }
}
