#pragma once

#include "Config.h"
#include "Socket.h"
#include "RecvBuffer.h"
#include "csmnet/util/ObjectPool.h"

#include <array>
#include <span>

#define DEFINE_PROCESS(iocp_event) virtual void Process(class iocp_event* event) { }
#define IOCP_EVENT_DEFAULT_IMPL() \
    using IocpEvent::IocpEvent; \
    void Process() override  { GetProcessor()->Process(this); }


namespace csmnet::detail
{
    class IIocpEventProcessor
    {
    public:
        virtual ~IIocpEventProcessor() noexcept = default;

        DEFINE_PROCESS(AcceptEvent)
        DEFINE_PROCESS(ConnectEvent)
        DEFINE_PROCESS(RecvEvent)
        DEFINE_PROCESS(SendEvent)
        DEFINE_PROCESS(DisconnectEvent)
    };

    class IocpEvent
    {
    public:
        static IocpEvent* From(OVERLAPPED* overlapped, uint32 byteTransferred)
        {
            if (overlapped == nullptr)
            {
                return nullptr;
            }

            IocpEvent* event = CONTAINING_RECORD(overlapped, IocpEvent, _overlapped);
            event->_bytesTransferred = byteTransferred;

            return event;
        }

        explicit IocpEvent(IIocpEventProcessor* processor) : _processor(processor) {}
        virtual ~IocpEvent() noexcept = default;

        virtual void Process() = 0;
        
        virtual void Reset() noexcept
        {
            ::memset(&_overlapped, 0, sizeof(_overlapped));
            _bytesTransferred = 0;
        }

        uint32 GetBytesTransferred() const noexcept { return _bytesTransferred; }

        uint32* GetBytesTransferredData() noexcept { return &_bytesTransferred; }
        const uint32* GetBytesTransferredData() const noexcept { return &_bytesTransferred; }
        OVERLAPPED* GetOverlapped() noexcept { return &_overlapped; }
        const OVERLAPPED* GetOverlapped() const noexcept { return &_overlapped; }

    protected:
        IIocpEventProcessor* GetProcessor() noexcept { return _processor; }
        
    private:
        IIocpEventProcessor* _processor = nullptr;
        uint32 _bytesTransferred = 0;
        OVERLAPPED _overlapped = {};
    };

    class AcceptEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()

        void Reset() noexcept override
        {
            IocpEvent::Reset();

            ::memset(_buffer.data(), 0, _buffer.size());
        }
        
        void PrepareSocket(Socket&& socket) noexcept
        {
            _acceptSocket = std::move(socket);
        }

        Socket& GetAcceptSocket() noexcept { return _acceptSocket; }
        const Socket& GetAcceptSocket() const noexcept { return _acceptSocket; }
        void* GetBuffer() noexcept { return _buffer.data(); }
        const void* GetBuffer() const noexcept { return _buffer.data(); }
        Endpoint GetRemote() noexcept
        {
            sockaddr_in* local = nullptr;
            socklen_t localAddrLen = sizeof(sockaddr_in);
            sockaddr_in* remote = nullptr;
            socklen_t remoteAddrLen = sizeof(sockaddr_in);

            ::GetAcceptExSockaddrs(
                _buffer.data(),
                _buffer.size(),
                sizeof(sockaddr_in) + 16,
                sizeof(sockaddr_in) + 16,
                reinterpret_cast<sockaddr**>(&local),
                &localAddrLen,
                reinterpret_cast<sockaddr**>(&remote),
                &remoteAddrLen);


            return Endpoint(*remote);
        }
    private:
        Socket _acceptSocket;
        std::array<char, 1024> _buffer;
    };

    class ConnectEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()
           
        void Reset() noexcept override
        {
            IocpEvent::Reset();
            _remote = Endpoint::Any(0);
        }
        
        void SetRemote(const Endpoint& remote) noexcept
        {
            _remote = remote;
        }

        const Endpoint& GetRemote() const noexcept
        {
            return _remote;
        }

    private:
        Endpoint _remote = Endpoint::Any(0);
    };

    class RecvEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()

        void Reset() noexcept override
        {
            IocpEvent::Reset();
            _wsaBuf.len = 0;
            _wsaBuf.buf = nullptr;
        }

        void SetData(char* buffer, const size_t size) noexcept
        {
            SetData(span(buffer, size));
        }

        void SetData(span<char> data) noexcept
        {
            _wsaBuf.buf = data.data();
            _wsaBuf.len = static_cast<ULONG>(data.size());
        }

        WSABUF* GetData() noexcept { return &_wsaBuf; }
        size_t GetBufferCount() const noexcept { return 1; }
    private:
        WSABUF _wsaBuf;
    };

    class SendEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()

        void Reset() noexcept override
        {
            IocpEvent::Reset();
            _wsaBuf.len = 0;
            _wsaBuf.buf = nullptr;
        }

        void SetData(span<char> data) noexcept
        {
            _wsaBuf.buf = data.data();
            _wsaBuf.len = static_cast<ULONG>(data.size());
        }

        WSABUF* GetData() noexcept { return &_wsaBuf; }
        size_t GetBufferCount() const noexcept { return 1; }
    private:
        WSABUF _wsaBuf;        
    };

    class DisconnectEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()
    };
}

