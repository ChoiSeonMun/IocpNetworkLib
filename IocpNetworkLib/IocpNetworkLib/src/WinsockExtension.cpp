#include "pch.h"
#include "csmnet/detail/WinsockExtension.h"

namespace csmnet::detail
{
    LPFN_CONNECTEX WinsockExtension::ConnectEx = nullptr;
    LPFN_DISCONNECTEX WinsockExtension::DisconnectEx = nullptr;

    void WinsockExtension::Init() noexcept
    {
        SOCKET dummySocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        BindWinsockFunc(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx));
        BindWinsockFunc(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx));
        ::closesocket(dummySocket);
    }
}
