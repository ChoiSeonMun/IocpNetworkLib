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

    class LibErrorCategory : public error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "csmnet";
        }

        string message(int value) const override
        {
            switch (static_cast<LibError>(value))
            {
            case LibError::FailToCreateIocpCore:
                return "Failed to create IOCP core";
            case LibError::FailToCreateAcceptor:
                return "Failed to create acceptor";
            case LibError::IocpCoreNotOpen:
                return "IOCP core is not open";
            case LibError::SocketAlreadyOpen:
                return "Socket is already open";
            case LibError::SocketNotOpen:
                return "Socket is not open";
            case LibError::SocketNotBound:
                return "Socket is not bound";
            case LibError::SocketNotListening:
                return "Socket is not listening";
            case LibError::SocketNotConnected:
                return "Socket is not connected";
            case LibError::CannotCreateSession:
                return "Cannot create session. Check the factory of the session.";
            case LibError::InvalidSocket:
                return "Invalid socket";
            case LibError::MemoryAllocationFailed:
                return "Memory allocation failed";
            case LibError::SessionClosed:
                return "Session is closed";
            default:
                return "Unknown library error";
            };
        }
    };

    const std::error_category& GetNetworkErrorCategory() noexcept
    {
        static NetworkCategory s_instance;
        return s_instance;
    }

    const std::error_category& GetLibErrorCategory() noexcept
    {
        static LibErrorCategory s_instance;
        return s_instance;
    }
}

