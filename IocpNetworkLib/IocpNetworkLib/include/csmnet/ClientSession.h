#pragma once

#include "csmnet/detail/Session.h"

namespace csmnet
{
    namespace detail { class IocpCore; }

    class ClientSession : public Session
    {
    public:
        using Session::Session;
        ClientSession(ClientSession&& other) noexcept = default;
        ClientSession& operator=(ClientSession&& other) noexcept = default;
        
        expected<void, error_code> Connect(const Endpoint& serverEndpoint, detail::IocpCore& iocpCore) noexcept;
        void Disconnect() noexcept override;

    protected:
        void OnRemoteClosed() override;
    private:
        void Process(detail::ConnectEvent* event) override;

    private:
        detail::ConnectEvent _connectEvent{ *this };
    };
}
