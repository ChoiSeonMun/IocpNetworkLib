#pragma once

#include "Config.h"
#include "IIocpRegistrable.h"
#include "IocpEvent.h"
#include "Socket.h"

namespace csmnet::util { class ILogger; }

namespace csmnet::detail
{
    class IServerApiForAcceptor;
    class IocpCore;
    class Acceptor final : public IIocpEventProcessor
    {
    public:
        Acceptor(util::ILogger& logger, IocpCore& iocpCore, IServerApiForAcceptor& server);
        Acceptor(const Acceptor&) = delete;
        Acceptor& operator=(const Acceptor&) = delete;
        Acceptor(Acceptor&&) = delete;
        Acceptor& operator=(Acceptor&&) = delete;
        ~Acceptor() noexcept
        {
            Close();
        }

        expected<void, error_code> Open(const Endpoint& local);
        void Close() noexcept;

        void Process(AcceptEvent* event) override;

    private:
        expected<void, error_code> PostAccept() noexcept;
    private:
        util::ILogger& _logger;
        IServerApiForAcceptor& _server;
        IocpCore& _iocpCore;
        
        Socket _listenSocket;
        AcceptEvent _acceptEvent{ *this };
    };
}