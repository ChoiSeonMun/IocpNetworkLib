#pragma once

#include "Config.h"
#include "Socket.h"

#include <array>

namespace csmnet::detail
{
    class IIocpEventProcessor
    {
    public:
        virtual ~IIocpEventProcessor() noexcept = default;
        virtual void Process(class AcceptEvent* event) {}
        virtual void Process(class ConnectEvent* event) {}
        virtual void Process(class RecvEvent* event) {}
        virtual void Process(class SendEvent* event) {}
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

        explicit IocpEvent(IIocpEventProcessor* processor);
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
        explicit AcceptEvent(IIocpEventProcessor* processor);

        void Process() override
        {
            GetProcessor()->Process(this);
        };

        void Reset() noexcept override
        {
            IocpEvent::Reset();
            
            _acceptSocket.Close();
            _acceptSocket.Open(SocketType::Tcp);

            ::memset(_buffer.data(), 0, _buffer.size());
        }

        Socket& GetAcceptSocket() noexcept { return _acceptSocket; }
        const Socket& GetAcceptSocket() const noexcept { return _acceptSocket; }
        void* GetBuffer() noexcept { return _buffer.data(); }
        const void* GetBuffer() const noexcept { return _buffer.data(); }
    private:
        Socket _acceptSocket;
        std::array<char, 1024> _buffer;
    };

    class ConnectEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            GetProcessor()->Process(this);
        };
    };

    class RecvEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            GetProcessor()->Process(this);
        };
    };

    class SendEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            GetProcessor()->Process(this);
        };
    };
}

