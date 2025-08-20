#include "pch.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/detail/IIocpRegistrable.h"
#include "csmnet/detail/IocpEvent.h"

#include <vector>

using namespace std;

namespace csmnet::detail
{
    void IocpCore::Register(const IIocpRegistrable& registrable) const noexcept
    {
        ::CreateIoCompletionPort(registrable.GetHandle(), _iocpHandle, 0, 0);
    }

    expected<IocpEvent*, error_code> IocpCore::GetQueuedCompletionEvent() const noexcept
    {
        OVERLAPPED* overlapped = nullptr;
        ULONG_PTR key = 0;
        DWORD bytesTransferred = 0;

        if (::GetQueuedCompletionStatus(_iocpHandle, &bytesTransferred, &key, &overlapped, INFINITE))
        {
            return IocpEvent::From(overlapped, bytesTransferred);
        }
        
        if (overlapped != nullptr)
        {
            return IocpEvent::From(overlapped, bytesTransferred);
        }

        return unexpected(error_code(::GetLastError(), system_category()));
    }

    expected<vector<IocpEvent*>, error_code> IocpCore::GetQueuedCompletionEvents() const noexcept
    {
        constexpr ULONG MAX_EVENTS_PER_CALL = 32;
        OVERLAPPED_ENTRY entries[MAX_EVENTS_PER_CALL];
        ULONG numEntriesRemoved = 0;

        if (FALSE == ::GetQueuedCompletionStatusEx(_iocpHandle, entries, MAX_EVENTS_PER_CALL, &numEntriesRemoved, INFINITE, FALSE))
        {
            auto lastError = ::GetLastError();
            if (lastError != WAIT_TIMEOUT)
            {
                return unexpected(error_code(lastError, system_category()));
            }
        }

        if (numEntriesRemoved == 0)
        {
            return {};
        }

        vector<IocpEvent*> events(numEntriesRemoved);
        for (size_t i = 0; i < numEntriesRemoved; ++i)
        {
            OVERLAPPED* overlapped = entries[i].lpOverlapped;
            DWORD bytesTransferred = entries[i].dwNumberOfBytesTransferred;
            events[i] = IocpEvent::From(overlapped, bytesTransferred);
        }

        return events;
    }

    expected<void, error_code> IocpCore::Open() noexcept
    {
        HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
        if (iocpHandle == NULL)
        {
            return unexpected(LibError::FailToCreateIocpCore);
        }
        _iocpHandle = iocpHandle;

        return {};
    }
}
