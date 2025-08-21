#pragma once

#include "Config.h"
#include "IIocpRegistrable.h"
#include "IocpEvent.h"
#include "Socket.h"

namespace csmnet::detail
{
    class IocpCore;

    class Acceptor final : public IIocpEventProcessor
    {
    public:
        Acceptor(IocpCore& iocpCore, const uint32 maxSessionCount);

        Acceptor(const Acceptor&) = delete;
        Acceptor& operator=(const Acceptor&) = delete;
        Acceptor(Acceptor&&) noexcept = default;
        Acceptor& operator=(Acceptor&&) noexcept = default;
        ~Acceptor() noexcept = default;

        expected<void, error_code> Open(const Endpoint& local);
        void Close() noexcept;

        void Process(AcceptEvent* event) override;

    private:
        expected<void, error_code> PostAccept() noexcept;
    private:
        uint32 _maxSessionCount;
        IocpCore& _iocpCore;
        Socket _listenSocket;
        AcceptEvent _acceptEvent;
    };
}