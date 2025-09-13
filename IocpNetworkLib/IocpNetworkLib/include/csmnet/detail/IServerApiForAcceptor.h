#pragma once

#include "csmnet/util/ObjectPool.h"

namespace csmnet::network { class ServerSession; }

namespace csmnet::detail
{
    class IServerApiForAcceptor
    {
    public:
        virtual ~IServerApiForAcceptor() noexcept = default;
        virtual util::PooledObject<csmnet::network::ServerSession> GetSession() noexcept = 0;
        virtual void AddSession(const util::PooledObject<csmnet::network::ServerSession>& session) noexcept = 0;
    };
}