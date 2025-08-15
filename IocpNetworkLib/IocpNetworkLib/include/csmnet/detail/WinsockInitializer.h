#pragma once

#pragma once

#include <Winsock2.h>
#include "WinsockExtension.h"

namespace csmnet::detail
{
    class WinsockInitializer
    {
    public:
        WinsockInitializer() noexcept
        {
            WSADATA wsaData;
            _result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
            
            WinsockExtension::Init();
        }

        WinsockInitializer(const WinsockInitializer&) = delete;
        WinsockInitializer& operator=(const WinsockInitializer&) = delete;
        WinsockInitializer(WinsockInitializer&&) = delete;
        WinsockInitializer& operator=(WinsockInitializer&&) = delete;
        ~WinsockInitializer() noexcept
        {
            ::WSACleanup();
        }
    private:
        int _result = 0;
    };

    // NOTE: 정적 변수는 main 함수가 호출되기 전에 초기화된다.
    inline static const WinsockInitializer winsockInitializerInstance;
}