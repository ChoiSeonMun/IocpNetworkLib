// IocpCore의 기능을 제공하는 IocpCore 클래스가 정의되어 있다.

#pragma once

#include "Config.h"

#include <vector>

namespace csmnet::detail
{
    class IIocpRegistrable;
    class IocpEvent;

    // Iocp의 핵심 기능을 제공한다.
    // 객체 생성 후 Open()을 호출해야 정상적으로 객체를 사용할 수 있다.
    // 사용이 전부 끝난 후 대기하고 있는 스레드를 안전하게 종료하기 위해서
    // Shutdown()을 호출한다. 이때, Shutdown()은 대기하고 있는 스레드 수만큼
    // 호출해야 한다. Close()를 명시적으로 호출할 필요는 없다.
    class IocpCore final
    {
    public:
        IocpCore() noexcept = default;
        ~IocpCore() noexcept
        {
            Close();
        }
        
        IocpCore(const IocpCore&) = delete;
        IocpCore& operator=(const IocpCore&) = delete;

        IocpCore(IocpCore&& other) noexcept
            : _iocpHandle(std::exchange(other._iocpHandle, INVALID_HANDLE_VALUE))
        {

        }

        IocpCore& operator=(IocpCore&& other) noexcept
        {
            if (this != &other)
            {
                Close();

                _iocpHandle = std::exchange(other._iocpHandle, INVALID_HANDLE_VALUE);
            }

            return *this;
        }
        
        void Register(const IIocpRegistrable& registrable) const noexcept;

        // 완료된 이벤트를 하나 가져온다.
        // `waitTimeMs`는 대기 시간(밀리초)이다. 기본값은 무한 대기(INFINITE)이다.
        // 반환 값은 nullptr일 수 있으며, 이는 종료한다는 의미다.
        // IocpCore가 열린 상태가 아닌 경우 LibError::IocpCoreNotOpen 오류를 반환한다.
        // 그 외 실패에 대해서는 GetLastError() 결과를 반환한다.
        expected<IocpEvent*, error_code> GetQueuedCompletionEvent(uint32 waitTimeMs = INFINITE) const noexcept;

        // 성공 값들 중에는 nullptr일 수 있다.
        expected<std::vector<IocpEvent*>, error_code> GetQueuedCompletionEvents() const noexcept;

        // 이벤트를 통지한다.
        // `iocpEvent`는 null일 수 있다. 이는 종료 신호로 사용될 수 있다.
        // 다만 PostQueuedCompletionEvent(nullptr)로 호출하기보다 Shutdown()을 사용하라.
        // IocpCore가 열린 상태가 아닌 경우 LibError::IocpCoreNotOpen 오류를 반환한다.
        // 그 외 실패에 대해서는 GetLastError() 결과를 반환한다.
        expected<void, error_code> PostQueuedCompletionEvent(IocpEvent* iocpEvent) const noexcept;

        expected<void, error_code> Open() noexcept;
        bool IsOpen() const noexcept { return _iocpHandle != INVALID_HANDLE_VALUE; }
        void Close() noexcept
        {
            if (IsOpen())
            {
                CloseHandle(_iocpHandle);
                _iocpHandle = INVALID_HANDLE_VALUE;
            }
        }

        // Iocp를 대기하고 있는 스레드에게 종료 신호를 보낸다.
        // 대기하고 있는 스레드 수만큼 호출해야 한다.
        expected<void, error_code> Shutdown() noexcept
        {
            return PostQueuedCompletionEvent(nullptr);
        }
    private:
        explicit IocpCore(HANDLE iocpHandle) noexcept
            : _iocpHandle(iocpHandle)
        {
        }
    private:
        HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
    };
}