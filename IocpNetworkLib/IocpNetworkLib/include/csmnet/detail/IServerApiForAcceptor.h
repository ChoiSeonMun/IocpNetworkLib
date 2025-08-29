#pragma once

#include "csmnet/util/ObjectPool.h"

namespace csmnet { class ServerSession; }

namespace csmnet::detail
{
    class IServerApiForAcceptor
    {
    public:
        virtual ~IServerApiForAcceptor() noexcept = default;
        virtual util::PooledObject<ServerSession> GetSession() noexcept = 0;
        virtual void AddSession(const util::PooledObject<ServerSession>& session) noexcept = 0;
    };
}