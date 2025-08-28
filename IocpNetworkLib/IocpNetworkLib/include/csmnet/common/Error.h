#pragma once

#include <WinSock2.h>
#include <system_error>

namespace csmnet::detail
{
    const std::error_category& GetNetworkErrorCategory() noexcept;
    const std::error_category& GetLibErrorCategory() noexcept;
}

namespace csmnet
{
    enum class NetworkError
    {
        AccessDenied = WSAEACCES,               // Permission denied
        AddressInUse = WSAEADDRINUSE,           // Address already in use
        AddressNotAvailable = WSAEADDRNOTAVAIL, // Cannot assign requested address
        AlreadyConnected = WSAEISCONN,          // Already connected
        AlreadyInProgress = WSAEALREADY,        // Operation already in progress
        BadAddress = WSAEFAULT,                 // The address is invalid
        BadDescriptor = WSAEBADF,               // Bad file descriptor
        ConnectionAborted = WSAECONNABORTED,    // Software caused connection abort
        ConnectionClosed = 0,                   // Connection closed
        ConnectionRefused = WSAECONNREFUSED,    // Connection refused
        ConnectionReset = WSAECONNRESET,        // Connection reset by peer
        HostDown = WSAEHOSTDOWN,                // Host is down
        HostUnreachable = WSAEHOSTUNREACH,      // No route to host
        InProgress = WSAEINPROGRESS,            // Operation now in progress (duplicate, consider removing)
        Interrupted = WSAEINTR,                 // Interrupted function call
        InvalidArgument = WSAEINVAL,            // Invalid argument passed
        MessageTooLong = WSAEMSGSIZE,           // Message too long
        NameTooLong = WSAENAMETOOLONG,          // Name too long
        NetworkDown = WSAENETDOWN,              // Network is down
        NetworkReset = WSAENETRESET,            // Network dropped connection on reset
        NetworkUnreachable = WSAENETUNREACH,    // Network is unreachable
        NoDescriptor = WSAEMFILE,               // No file descriptor available
        NoMemory = WSAENOBUFS,                  // No buffer space available
        NoProtocolOption = WSAENOPROTOOPT,      // Protocol option not available
        NotConnected = WSAENOTCONN,             // Socket is not connected
        NotSocket = WSAENOTSOCK,                // Descriptor is not a socket
        OperationInProgress = WSAEINPROGRESS,   // Operation now in progress
        OperationNotSupported = WSAEOPNOTSUPP,  // Operation not supported
        Shutdown = WSAESHUTDOWN,                // Cannot send after socket shutdown
        TimedOut = WSAETIMEDOUT,                // Connection timed out
        TooManyRefs = WSAETOOMANYREFS,          // Too many references
        WouldBlock = WSAEWOULDBLOCK,            // Operation would block
    };

    enum class LibError
    {
        FailToCreateIocpCore = 1,
        FailToCreateAcceptor,
        IocpCoreNotOpen,
        SocketAlreadyOpen,
        InvalidSocket,
        SocketNotOpen,
        SocketNotBound,
        SocketNotListening,
        SocketNotConnected,
        CannotCreateSession,
        MemoryAllocationFailed,
        SessionClosed,
    };

    inline std::error_code make_error_code(NetworkError e) noexcept
    {
        return std::error_code(static_cast<int>(e), detail::GetNetworkErrorCategory());
    }

    inline std::error_code make_error_code(LibError e) noexcept
    {
        return std::error_code(static_cast<int>(e), detail::GetLibErrorCategory());
    }

    inline NetworkError TranslateWsaError(int wsaError) noexcept
    {
        return static_cast<NetworkError>(wsaError);
    }
}

namespace std
{
    template<> struct is_error_code_enum<csmnet::NetworkError> : true_type {};
    template<> struct is_error_code_enum<csmnet::LibError> : true_type {};
}