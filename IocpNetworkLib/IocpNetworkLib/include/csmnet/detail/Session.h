#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/Socket.h"
#include "csmnet/detail/Endpoint.h"
#include "csmnet/detail/IocpEvent.h"
#include "csmnet/util/FifoBuffer.h"

namespace csmnet::util { class ILogger; }

namespace csmnet
{
    class Session : public detail::IIocpEventProcessor
    {
        static constexpr size_t kMinRecvSize = 1024;
    public:
        explicit Session(util::ILogger& logger) noexcept;
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;
        Session(Session&& other) noexcept;
        Session& operator=(Session&& other) noexcept;

        virtual ~Session() = default;

        bool IsConnected() const noexcept { return _isConnected; }
        const Endpoint& GetRemoteEndpoint() const noexcept { return _remote; }

        void SetSendBufferSize(size_t size) { _sendBuffer.Resize(size); }
        void SetRecvBufferSize(size_t size) { _recvBuffer.Resize(size); }

        expected<void, error_code> Activate() noexcept;

        // 송신 버퍼를 가져온다.
        // 다른 세션에서 송신 버퍼를 사용 중이라면 송신 버퍼를 얻을 때까지 대기한다.
        // TODO: 병목 지점이기에 개선이 필요하다.
        util::FifoBuffer<byte>& GetSendBuffer() noexcept
        {
            bool expected = false;
            bool desired = true;
            while (_isSending.compare_exchange_strong(expected, desired) == false)
            {
                expected = false;
            }

            return _sendBuffer;
        }

        virtual expected<void, error_code> Send(util::FifoBuffer<byte>& buffer) noexcept;
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
        expected<void, error_code> PostSend(util::FifoBuffer<csmnet::byte>& buffer) noexcept;
    protected:
        std::atomic<bool> _isConnected{ false };
        util::ILogger& _logger;
        detail::Socket _socket;
        Endpoint _remote = Endpoint::Any(0);
    
    private:    
        detail::SendEvent _sendEvent{ *this };
        util::FifoBuffer<csmnet::byte> _sendBuffer;
        std::atomic<bool> _isSending{ false };
        //util::ObjectPool<detail::FifoBuffer> _sendBufferPool;
        //std::mutex _sendMutex;

        detail::RecvEvent _recvEvent{ *this };
        util::FifoBuffer<csmnet::byte> _recvBuffer;
    };
}
