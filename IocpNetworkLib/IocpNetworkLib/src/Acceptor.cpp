#include "pch.h"
#include "csmnet/detail/Acceptor.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/Server.h"
#include "csmnet/util/ILogger.h"

#include <print>

namespace csmnet::detail
{
    Acceptor::Acceptor(util::ILogger& logger, IocpCore& iocpCore, IServerApiForAcceptor& server)
        :
        _logger(logger),
        _iocpCore(iocpCore),
        _server(server)
    {
        CSM_ASSERT(_iocpCore.IsOpen());
    }

    expected<void, error_code> Acceptor::Open(const Endpoint& local)
    {
        return _listenSocket.Open()
            .and_then([this, &local]() -> expected<void, error_code>
                {
                    _listenSocket.SetOption(Socket::Linger{ true, 0 });
                    _listenSocket.SetOption(Socket::ReuseAddress{ true });
                    return _listenSocket.Bind(local);
                })
            .and_then([this, &local]() -> expected<void, error_code>
                {
                    return _listenSocket.Listen();
                })
            .and_then([this]() -> expected<void, error_code>
                {
                    _iocpCore.Register(_listenSocket);
                    return PostAccept();
                });
    }

    void Acceptor::Close() noexcept
    {
        _listenSocket.Close();
    }

    void Acceptor::Process(AcceptEvent* event)
    {
        Socket acceptedSocket = std::move(event->GetAcceptSocket());
        acceptedSocket.SetOption(Socket::UpdateAcceptContext{ _listenSocket });
        auto remote = acceptedSocket.GetRemoteEndpoint();

        auto session = _server.GetSession();
        if (session)
        {
            _iocpCore.Register(acceptedSocket);
            session->SetConnection(std::move(acceptedSocket), std::move(*remote));
            _server.AddSession(session);

            session->OnConnected();
        }
        else
        {
            _logger.Info(format("Acceptor::Process - No available session. Closing {}:{}", remote->GetIp(), remote->GetPort()));
        }
        
        if (auto result = PostAccept(); !result)
        {
            _logger.Error(format("Acceptor::Process - Fail to accept: [{}] {}", result.error().value(), result.error().message()));
            _logger.Error(format("Acceptor::Process - Stopping accepting new connections."));
            Close();
        }
    }

    expected<void, error_code> Acceptor::PostAccept() noexcept
    {
        Socket acceptSocket;
        return acceptSocket.Open()
            .and_then([this, &acceptSocket]()
                {
                    _acceptEvent.Reset(std::move(acceptSocket));
                    return _listenSocket.AcceptEx(_acceptEvent);
                });
    }
}



