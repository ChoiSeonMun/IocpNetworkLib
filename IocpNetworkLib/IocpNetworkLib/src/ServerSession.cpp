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

    void ServerSession::Disconnect() noexcept
    {
        if (_isConnected.exchange(false) == false)
        {
            return;
        }

        _logger.Info(format("ServerSession::Disconnect - [Session {}:{}] Disconnecting...", _remote.GetIp(), _remote.GetPort()));

        _socket.Shutdown(ShutdownKind::Both);
        _socket.Close();

        OnDisconnected();
    }

    void ServerSession::OnRemoteClosed()
    {
        _logger.Info(format("[Session {}:{}] Remote disconnected.", _remote.GetIp(), _remote.GetPort()));

        Disconnect();
    }
    
}