#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/Socket.h"
#include "csmnet/Endpoint.h"
#include "csmnet/detail/IocpEvent.h"
#include "csmnet/detail/FifoBuffer.h"

namespace csmnet::util { class ILogger; }

namespace csmnet
{
    class Session : public detail::IIocpEventProcessor
    {
    public:
        explicit Session(util::ILogger& logger) noexcept;
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;
        Session(Session&& other) noexcept;
        Session& operator=(Session&& other) noexcept;

        virtual ~Session() = default;

        bool IsConnected() const noexcept { return _isConnected; }
        const Endpoint& GetRemoteEndpoint() const noexcept { return _remote; }
        void SetRecvBufferSize(size_t size) { _recvBuffer.Resize(size); }

        expected<void, error_code> Activate() noexcept;
        virtual expected<void, error_code> Send(std::span<const std::byte> message) noexcept;
        virtual void Disconnect() noexcept = 0;

        virtual void OnConnected() = 0;
        virtual void OnDisconnected() = 0;
        virtual void OnRecv(std::span<const std::byte> data) = 0;

    protected:
        virtual void OnRemoteClosed() = 0;

    private:
        void Process(detail::RecvEvent* event) final;
        void Process(detail::SendEvent* event) final;

        expected<void, error_code> PostRecv() noexcept;
        expected<void, error_code> PostSend(std::span<const std::byte> message) noexcept;
    protected:
        std::atomic<bool> _isConnected{ false };
        util::ILogger& _logger;
        detail::Socket _socket;
        Endpoint _remote = Endpoint::Any(0);
    
    private:    
        detail::SendEvent _sendEvent{ *this };
        detail::RecvEvent _recvEvent{ *this };
        detail::FifoBuffer _recvBuffer;
    };
}
