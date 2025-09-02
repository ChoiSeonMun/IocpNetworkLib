#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include "WinsockInitializer.h"

#include <utility>

#include "csmnet/common/Type.h"
#include "csmnet/common/Error.h"
#include "csmnet/common/Assertion.h"
