#pragma once

#include "csmnet/detail/Session.h"

namespace csmnet::detail { class IocpCore; class Endpoint; class ConnectEvent; }

namespace csmnet::network
{
    class ClientSession : public csmnet::detail::Session
    {
    public:
        using Session::Session;
        ClientSession(ClientSession&& other) noexcept = default;
        ClientSession& operator=(ClientSession&& other) noexcept = default;
        
        expected<void, error_code> Connect(const csmnet::detail::Endpoint& serverEndpoint, csmnet::detail::IocpCore& iocpCore) noexcept;
        void Disconnect() noexcept override;

    protected:
        void OnRemoteClosed() override;
    private:
        void Process(csmnet::detail::ConnectEvent* event) override;

    private:
        csmnet::detail::ConnectEvent _connectEvent{ *this };
    };
}
