#pragma once

#include "Config.h"

namespace csmnet::detail
{
    class IIocpEventProcessor
    {
    public:
        virtual ~IIocpEventProcessor() noexcept = default;
        virtual void Process(class AcceptEvent* event) = 0;
        virtual void Process(class RecvEvent* event) = 0;
        virtual void Process(class SendEvent* event) = 0;
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

        explicit IocpEvent(IIocpEventProcessor* visitor) : _processor(visitor) {}
        virtual ~IocpEvent() noexcept = default;

        virtual void Process() = 0;
        
        void Reset() noexcept
        {
            ::memset(&_overlapped, 0, sizeof(_overlapped));
            _bytesTransferred = 0;
        }

        uint32 GetByteTransferred() const noexcept { return _bytesTransferred; }
        
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
        using IocpEvent::IocpEvent;

        void Process() override
        {
            GetProcessor()->Process(this);
            Reset();
        };
    };

    class RecvEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            GetProcessor()->Process(this);
            Reset();
        };
    };

    class SendEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            GetProcessor()->Process(this);
            Reset();
        };
    };
}

