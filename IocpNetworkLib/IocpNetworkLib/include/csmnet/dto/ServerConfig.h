#pragma once

#include "csmnet/common/Type.h"

namespace csmnet::dto
{
    struct ServerConfig
    {
        uint16 Port;
        uint32 MaxSessionCount;
        uint32 IoThreadCount;
        uint32 SessionCleanIntervalSec;
    };
}