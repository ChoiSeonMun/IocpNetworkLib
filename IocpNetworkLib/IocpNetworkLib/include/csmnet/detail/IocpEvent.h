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
    void Process() override  { _processor->Process(this); }


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

        uint32 GetBytesTransferred() const noexcept { return _bytesTransferred; }

        uint32* GetBytesTransferredData() noexcept { return &_bytesTransferred; }
        OVERLAPPED* GetOverlappedData() noexcept { return &_overlapped; }
    protected:
        void Reset() noexcept
        {
            ::memset(&_overlapped, 0, sizeof(_overlapped));
            _bytesTransferred = 0;
        }

    protected:
        IIocpEventProcessor* _processor = nullptr;
        
    private:
        uint32 _bytesTransferred = 0;
        OVERLAPPED _overlapped = {};
    };

    class AcceptEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()

        void Reset(Socket&& socket) noexcept
        {
            CSM_ASSERT(socket.IsValid());

            IocpEvent::Reset();
            _acceptSocket = std::move(socket);
            ::memset(_buffer.data(), 0, _buffer.size());
        }
        
        Socket& GetAcceptSocket() noexcept { return _acceptSocket; }
        void* GetBuffer() noexcept { return _buffer.data(); }
    private:
        Socket _acceptSocket;
        std::array<char, 1024> _buffer;
    };

    class ConnectEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()
           
        void Reset(const Endpoint& remote) noexcept
        {
            IocpEvent::Reset();
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

        void Reset(span<byte> buffer) noexcept
        {
            CSM_ASSERT(buffer.data() != nullptr);
            CSM_ASSERT(buffer.empty() == false);

            IocpEvent::Reset();

            _wsaBuf.len = static_cast<ULONG>(buffer.size());
            _wsaBuf.buf = reinterpret_cast<char*>(buffer.data());
        }

        uint32* GetFlagsData() noexcept { return &_flags; }
        WSABUF* GetData() noexcept { return &_wsaBuf; }
        size_t GetBufferCount() const noexcept { return 1; }
    private:
        uint32 _flags{ 0 };
        WSABUF _wsaBuf;
    };

    class SendEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL()

        void Reset(span<byte> buffer) noexcept
        {
            CSM_ASSERT(buffer.data() != nullptr);
            CSM_ASSERT(buffer.empty() == false);

            IocpEvent::Reset();
            _wsaBuf.len = static_cast<ULONG>(buffer.size());
            _wsaBuf.buf = reinterpret_cast<char*>(buffer.data());
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

        void Reset() noexcept
        {
            IocpEvent::Reset();
        }
    };
}

