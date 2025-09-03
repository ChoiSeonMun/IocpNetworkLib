#include "pch.h"
#include "csmnet/ServerSession.h"
#include "csmnet/util/ILogger.h"

using namespace std;
using namespace csmnet::detail;

namespace csmnet
{
    void ServerSession::SetConnection(detail::Socket&& socket, Endpoint&& remote)
    {
        _socket = std::move(socket);
        _remote = std::move(remote);
        _isConnected = true;
    }

    expected<void, error_code> ServerSession::Start() noexcept
    {
        return PostRecv()
            .and_then([this]() -> expected<void, error_code>
                {
                    _logger.Info(format("ServerSession::Start - [Session {}:{}] Started.", _remote.GetIp(), _remote.GetPort()));
                    OnConnected();

                    return {};
                });
    }

    expected<void, error_code> ServerSession::Send(std::span<const std::byte> data) noexcept
    {
        if (_isConnected == false)
        {
            return unexpected(LibError::SessionClosed);
        }

        // TODO: Send 구현
    }

    void ServerSession::Disconnect() noexcept
    {
        if (_isConnected)
        {
            _logger.Info(format("ServerSession::Disconnect - [Session {}:{}] Disconnecting...", _remote.GetIp(), _remote.GetPort()));

            _socket.Shutdown(ShutdownKind::Both);
            _socket.Close();

            OnDisconnected();

            _isConnected = false;
        }
    }

    void ServerSession::Process(detail::RecvEvent* event)
    {
        if (event->GetBytesTransferred() == 0)
        {
            _logger.Info(format("ServerSession::Process(RecvEvent) - [Session {}:{}] Remote disconnected.", _remote.GetIp(), _remote.GetPort()));
            
            Disconnect();
            return;
        }
        else
        {
            if (auto result = PostRecv(); !result)
            {
                _logger.Error(format("ServerSession::Process(RecvEvent) - [Session {}:{}] Fail to post recv: [{}] {}",
                    _remote.GetIp(),
                    _remote.GetPort(),
                    result.error().value(),
                    result.error().message()));
            }
        }
        
    }

    expected<void, error_code> ServerSession::PostRecv() noexcept
    {
        const auto buffer = _recvBuffer.GetWritableSpan();

        _recvEvent.Reset(buffer);
        return _socket.RecvEx(_recvEvent);
    }
}