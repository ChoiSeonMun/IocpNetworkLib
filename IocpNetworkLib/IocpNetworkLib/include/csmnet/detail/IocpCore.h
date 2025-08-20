#pragma once

#include "Config.h"

#include <vector>

namespace csmnet::detail
{
    class IIocpRegistrable;
    class IocpEvent;

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

        // 성공 값은 nullptr일 수 있다.
        expected<IocpEvent*, error_code> GetQueuedCompletionEvent() const noexcept;
        // 성공 값들 중에는 nullptr일 수 있다.
        expected<std::vector<IocpEvent*>, error_code> GetQueuedCompletionEvents() const noexcept;

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
    private:
        explicit IocpCore(HANDLE iocpHandle) noexcept
            : _iocpHandle(iocpHandle)
        {
        }
    private:
        HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
    };
}