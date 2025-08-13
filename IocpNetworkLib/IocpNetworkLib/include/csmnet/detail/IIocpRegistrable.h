#pragma once

#include "Config.h"

namespace csmnet::detail
{
    class IIocpRegistrable
    {
    public:
        virtual ~IIocpRegistrable() noexcept = default;
        virtual HANDLE GetHandle() const noexcept = 0;
    };
}