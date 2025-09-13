#pragma once

#include "csmnet/common/Type.h"

#include <string>

namespace csmnet::dto
{
    struct ClientConfig
    {
        uint32 IoThreadCount;
        uint32 SessionCount;
        uint16 ServerPort;
        std::string ServerIp;
    };
}