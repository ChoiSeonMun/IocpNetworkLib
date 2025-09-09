#include "pch.h"
#include "csmnet/ClientSession.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/util/ILogger.h"

using namespace std;
using namespace csmnet::detail;

namespace csmnet
{
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

    void ClientSession::OnRemoteClosed()
    {
        _socket.Shutdown(ShutdownKind::Receive);
        _socket.Close();
    }

    void ClientSession::Process(detail::ConnectEvent* event)
    {
        _socket.SetOption(Socket::UpdateConnectContext{});
        _remote = event->GetRemote();
        
        if (auto result = Activate(); !result)
        {
            _logger.Error(format("ClientSession::Process(ConnectEvent) - fail to activate : [{}] {}",
                result.error().value(),
                result.error().message()));

            Disconnect();
        }
    }
}