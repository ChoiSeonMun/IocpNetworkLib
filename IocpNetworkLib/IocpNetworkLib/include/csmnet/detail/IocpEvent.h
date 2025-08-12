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
        explicit IocpEvent(IIocpEventProcessor* visitor) : _processor(visitor) {}
        virtual ~IocpEvent() noexcept = default;

        virtual void Process() = 0;
        
        void Reset() noexcept
        {
            ::memset(&_overlapped, 0, sizeof(_overlapped));
        }

        OVERLAPPED* GetOverlapped() noexcept { return &_overlapped; }
        const OVERLAPPED* GetOverlapped() const noexcept { return &_overlapped; }
    protected:
        OVERLAPPED _overlapped = {};
        IIocpEventProcessor* _processor = nullptr;
    };

    class AcceptEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            _processor->Process(this);
            Reset();
        };
    };

    class RecvEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            _processor->Process(this);
            Reset();
        };
    };

    class SendEvent final : public IocpEvent
    {
    public:
        using IocpEvent::IocpEvent;

        void Process() override
        {
            _processor->Process(this);
            Reset();
        };
    };
}