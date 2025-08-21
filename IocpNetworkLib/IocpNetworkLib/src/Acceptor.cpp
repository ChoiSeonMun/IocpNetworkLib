#include "pch.h"
#include "csmnet/detail/Acceptor.h"
#include "csmnet/detail/IocpCore.h"

#include <print>

namespace csmnet::detail
{
    expected<void, error_code> Acceptor::Open(const Endpoint& local)
    {
        return _listenSocket.Open(SocketType::Tcp)
            .and_then([this, &local]() -> expected<void, error_code>
                {
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

    Acceptor::Acceptor(IocpCore& iocpCore, const uint32 maxSessionCount)
        : _iocpCore(iocpCore), _maxSessionCount(maxSessionCount), _acceptEvent(this)
    {
    }

    void Acceptor::Close() noexcept
    {
        _listenSocket.Close();
    }

    void Acceptor::Process(AcceptEvent* event)
    {
        // TODO: 연결 처리.
        println("New connection accepted");
        
        PostAccept();
    }

    expected<void, error_code> Acceptor::PostAccept() noexcept
    {
        _acceptEvent.Reset();
        return _listenSocket.AcceptEx(_acceptEvent);    
    }
}



