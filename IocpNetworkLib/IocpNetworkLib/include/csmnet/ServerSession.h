#pragma once

#include "csmnet/detail/Session.h"
#include "csmnet/detail/IocpEvent.h"

namespace csmnet::util { class ILogger; }

namespace csmnet
{
    class ServerSession : public Session, public detail::IIocpEventProcessor
    {
    public:
        explicit ServerSession(util::ILogger& logger) noexcept
            : _logger(logger)
        {

        }
        
        ServerSession(ServerSession&& other) noexcept
            : Session(std::move(other)),
            _logger(other._logger),
            _recvEvent(*this),
            _recvBuffer(std::move(other._recvBuffer))
        {

        }
        ServerSession& operator=(ServerSession&& other) noexcept
        {
            if (this != &other)
            {
                Session::operator=(std::move(other));
                _recvBuffer = std::move(other._recvBuffer);
            }
            
            return *this;
        }

        void SetRecvBufferSize(size_t size) { _recvBuffer.Resize(size); }
        void SetConnection(detail::Socket&& socket, Endpoint&& remote);

        expected<void, error_code> Start() noexcept;
        expected<void, error_code> Send(std::span<const std::byte> data) noexcept override;
        void Disconnect() noexcept override;
    private:
        void Process(detail::RecvEvent* event) override;

        expected<void, error_code> PostRecv() noexcept;

    protected:
        util::ILogger& _logger;

    private:
        detail::RecvEvent _recvEvent{ *this };
        detail::RecvBuffer _recvBuffer;
    };
}
