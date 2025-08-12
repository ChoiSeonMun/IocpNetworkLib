#include "pch.h"
#include "csmnet/common/Error.h"

namespace csmnet::detail
{
    using namespace std;

    class LocalFreeOnBlockExit final
    {
    public:
        explicit LocalFreeOnBlockExit(void* ptr) : _ptr(ptr) {}
        ~LocalFreeOnBlockExit() noexcept
        {
            if (_ptr)
            {
                ::LocalFree(_ptr);
            }
        }
    private:
        void* _ptr;
    };

    class NetworkCategory final : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "csmnet.network";
        }

        string message(int value) const override
        {
            char* msg = 0;
            DWORD length = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS, 0, value,
                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (char*)&msg, 0, 0);
            LocalFreeOnBlockExit localFreeObj(msg);
            if (length && msg[length - 1] == '\n')
            {
                msg[--length] = '\0';
            }
            if (length && msg[length - 1] == '\r')
            {
                msg[--length] = '\0';
            }

            return (length > 0) ? msg : "Unknown network error";
        }
    };

    const std::error_category& GetNetworkErrorCategory() noexcept
    {
        static NetworkCategory instance;
        return instance;
    }
}

