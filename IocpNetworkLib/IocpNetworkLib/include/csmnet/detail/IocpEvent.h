#pragma once

#include "Config.h"
#include "Socket.h"
#include "csmnet/util/ObjectPool.h"
#include "csmnet/util/FifoBuffer.h"

#include <array>
#include <span>

#define DEFINE_PROCESS(iocp_event) virtual void Process(class iocp_event* event) { }
#define IOCP_EVENT_DEFAULT_IMPL(iocpEvent) \
    using IocpEvent::IocpEvent; \
    void Process() override  { _processor.Process(this); } \
    std::string_view ToString() override { return #iocpEvent; }


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

        explicit IocpEvent(IIocpEventProcessor& processor) : _processor(processor) {}
        IocpEvent(const IocpEvent&) = delete;
        IocpEvent& operator=(const IocpEvent&) = delete;
        IocpEvent(IocpEvent&&) = delete;
        IocpEvent& operator=(IocpEvent&&) = delete;
        virtual ~IocpEvent() noexcept = default;

        virtual void Process() = 0;
        virtual std::string_view ToString() = 0;

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
        IIocpEventProcessor& _processor;
        
    private:
        uint32 _bytesTransferred = 0;
        OVERLAPPED _overlapped = {};
    };

    class AcceptEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL(AcceptEvent)

        void Reset(Socket&& socket) noexcept
        {
            CSM_ASSERT(socket.IsOpen());

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
        IOCP_EVENT_DEFAULT_IMPL(ConnectEvent)
           
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
        IOCP_EVENT_DEFAULT_IMPL(RecvEvent)

        void Reset(span<byte> buffer) noexcept
        {
            CSM_ASSERT(buffer.data() != nullptr);
            CSM_ASSERT(buffer.empty() == false);

            IocpEvent::Reset();

            _wsaBuf.len = static_cast<ULONG>(buffer.size());
            _wsaBuf.buf = reinterpret_cast<char*>(buffer.data());
        }

        // 수신된 바이트 수를 반환한다.
        size_t GetRecvByte() const noexcept
        {
            return GetBytesTransferred();
        }

        bool IsRemoteClosed() const noexcept { return GetBytesTransferred() == 0; }
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
        IOCP_EVENT_DEFAULT_IMPL(SendEvent)

        void Reset() noexcept
        {
            IocpEvent::Reset();

            _wsabufs.clear();
            _sendBuffers.clear();
        }

        // SendBuffer 추가
        // 내부적으로 std::vector를 사용하고 있어 이와 관련된 예외가 발생할 수 있다.
        void AddSendBuffer(util::FifoBuffer<byte>* sendBuffer)
        {
            CSM_ASSERT(sendBuffer != nullptr);

            _sendBuffers.push_back(sendBuffer);
            
            const auto readableSpan = sendBuffer->Peek();
            WSABUF wsabuf;
            wsabuf.buf = reinterpret_cast<char*>(const_cast<byte*>((readableSpan.data())));
            wsabuf.len = static_cast<ULONG>(readableSpan.size());
            _wsabufs.push_back(wsabuf);
        }

        // SendBuffer 모두 제거
        // 이벤트가 완료된 후 반드시 호출해야 한다.
        void ClearSendBuffers() noexcept
        {
            _sendBuffers.clear();
        }

        WSABUF* GetData() noexcept { return _wsabufs.data(); }
        size_t GetBufferCount() const noexcept { return _wsabufs.size(); }
    private:
        std::vector<WSABUF> _wsabufs;
        std::vector<util::FifoBuffer<byte>*> _sendBuffers;
    };

    class DisconnectEvent final : public IocpEvent
    {
    public:
        IOCP_EVENT_DEFAULT_IMPL(DisconnectEvent)

        void Reset() noexcept
        {
            IocpEvent::Reset();
        }
    };
}

