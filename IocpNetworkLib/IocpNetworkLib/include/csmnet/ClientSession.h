#pragma once

#include "csmnet/detail/Session.h"
#include "csmnet/detail/IocpEvent.h"

namespace csmnet
{
    namespace detail { class IocpCore; }
    namespace util { class ILogger; }

    class ClientSession : public Session, public detail::IIocpEventProcessor
    {
    public:
        explicit ClientSession(util::ILogger& logger) noexcept
            : _logger(logger)
        {
        }

        ClientSession(const ClientSession&) = delete;
        ClientSession& operator=(const ClientSession&) = delete;
        ClientSession(ClientSession&& other) noexcept
            : Session(std::move(other)),
            _logger(other._logger)
        {

        }
        ClientSession& operator=(ClientSession&& other) noexcept
        {
            if (this != &other)
            {
                Session::operator=(std::move(other));
            }
            
            return *this;
        }
        
        expected<void, error_code> Send(std::span<const std::byte> data) noexcept override;
        expected<void, error_code> Connect(const Endpoint& serverEndpoint, detail::IocpCore& iocpCore) noexcept;
        void Disconnect() noexcept override;

        void SetRecvBufferSize(size_t size) { _recvBuffer.Resize(size); }

        const Endpoint& GetRemoteEndpoint() const noexcept { return _remote; }
    private:
        void Process(detail::ConnectEvent* event) override;
        void Process(detail::RecvEvent* event) override;

        expected<void, error_code> PostRecv() noexcept;

    protected:
        util::ILogger& _logger;

    private:
        detail::ConnectEvent _connectEvent{ *this };
        detail::SendEvent _sendEvent{ *this };
        detail::RecvEvent _recvEvent{ *this };
        detail::RecvBuffer _recvBuffer;
    };
}
