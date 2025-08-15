#include "pch.h"
#include "csmnet/detail/WinsockExtension.h"

namespace csmnet::detail
{
    LPFN_CONNECTEX WinsockExtension::ConnectEx = nullptr;

    void WinsockExtension::Init() noexcept
    {
        SOCKET dummySocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        BindWinsockFunc(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx));
        ::closesocket(dummySocket);
    }
}
