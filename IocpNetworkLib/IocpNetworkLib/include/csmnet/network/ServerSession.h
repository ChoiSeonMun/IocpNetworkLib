#pragma once

#include "csmnet/detail/Session.h"

namespace csmnet::network
{
    class ServerSession : public csmnet::detail::Session
    {
    public:
        using Session::Session;
        ServerSession(ServerSession&& other) noexcept = default;
        ServerSession& operator=(ServerSession&& other) noexcept = default;

        void SetConnection(detail::Socket&& socket, detail::Endpoint&& remote);
        void Disconnect() noexcept override;

    protected:
        void OnRemoteClosed() override;
    };
}
