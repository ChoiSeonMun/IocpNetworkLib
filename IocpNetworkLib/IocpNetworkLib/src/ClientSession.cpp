#include "pch.h"
#include "csmnet/ClientSession.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/util/ILogger.h"

using namespace std;
using namespace csmnet::detail;

namespace csmnet
{
    expected<void, error_code> ClientSession::Send(std::span<const std::byte> data) noexcept
    {
        // TODO: Send 등록
        return expected<void, error_code>();
    }

    expected<void, error_code> ClientSession::Connect(const Endpoint& serverEndpoint, IocpCore& iocpCore) noexcept
    {
        CSM_ASSERT(iocpCore.IsOpen());

        return _socket.Open()
            .and_then([this, &iocpCore]()
                {
                    iocpCore.Register(_socket);
                    return _socket.Bind(Endpoint::Any(0));
                })
            .and_then([this, &serverEndpoint]()
                {
                    _connectEvent.Reset(serverEndpoint);
                    return _socket.ConnectEx(_connectEvent);
                });
    }

    void ClientSession::Disconnect() noexcept
    {
        if (bool wasConnected = _isConnected.exchange(false); wasConnected == false)
        {
            return;
        }

        if (auto result = _socket.Shutdown(ShutdownKind::Send); !result)
        {
            _logger.Error(format("ClientSession::Disconnect - fail to shutdown : [{}] {}",
                result.error().value(),
                result.error().message()));
            _socket.Close();
        }
    }

    void ClientSession::Process(detail::ConnectEvent* event)
    {
        _socket.SetOption(Socket::UpdateConnectContext{});
        _remote = event->GetRemote();
        _isConnected = true;

        if (auto result = PostRecv(); !result)
        {
            _logger.Error(format("ClientSession::Process(ConnectEvent) - fail to post recv : [{}] {}",
                result.error().value(),
                result.error().message()));

            Disconnect();
            return;
        }

        _logger.Info("ClientSession::Process(ConnectEvent) - Connected");
        OnConnected();
    }

    expected<void, error_code> ClientSession::PostRecv() noexcept
    {
        const auto buffer = _recvBuffer.GetWritableSpan();

        _recvEvent.Reset(buffer);
        return _socket.RecvEx(_recvEvent);
    }

    void ClientSession::Process(detail::RecvEvent* event)
    {
        if (event->GetBytesTransferred() == 0)
        {
            _socket.Shutdown(ShutdownKind::Receive);
            _socket.Close();
        }
    }
}