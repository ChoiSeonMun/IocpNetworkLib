#pragma once

#include "csmnet/detail/Session.h"

namespace csmnet
{
    class ServerSession : public Session
    {
    public:
        using Session::Session;
        ServerSession(ServerSession&& other) noexcept = default;
        ServerSession& operator=(ServerSession&& other) noexcept = default;

        void SetConnection(detail::Socket&& socket, Endpoint&& remote);
        void Disconnect() noexcept override;

    protected:
        void OnRemoteClosed() override;
    };
}
