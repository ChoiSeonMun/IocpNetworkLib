#pragma once

#include <WinSock2.h>
#include <MSWSock.h>

namespace csmnet::detail
{
    class WinsockExtension
    {
        friend class WinsockInitializer;
    public:
        static LPFN_CONNECTEX ConnectEx;
        static LPFN_DISCONNECTEX DisconnectEx;

    private:
        static void Init() noexcept;
        static void BindWinsockFunc(SOCKET socket, GUID guid, LPVOID* fn)
        {
            DWORD bytes = 0;
            WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT & bytes, NULL, NULL);
        }
    };
}