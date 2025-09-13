#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/dto/ClientConfig.h"

#include <thread>
#include <functional>
#include <vector>
#include <string>

namespace csmnet::detail { class Endpoint; }

namespace csmnet::network
{
    class ClientSession;
    
    using ClientSessionFactory = std::function<std::unique_ptr<ClientSession>()>;
    class Client
    {
    public:
        Client(ClientSessionFactory sessionFactory, csmnet::dto::ClientConfig config) noexcept;
        virtual ~Client() noexcept;
        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;
        Client(Client&&) noexcept = delete;
        Client& operator=(Client&&) noexcept = delete;

        bool IsRunning() const noexcept { return _isRunning; }
        virtual expected<void, error_code> Run() noexcept;
        virtual void Stop() noexcept;
    private:
        void DispatchIO();
    private:
        std::atomic<bool> _isRunning{ false };
        csmnet::dto::ClientConfig _config;
        detail::IocpCore _iocpCore;
        ClientSessionFactory _sessionFactory;
        std::vector<std::jthread> _ioThreads;
        std::vector<std::unique_ptr<ClientSession>> _sessions;
    };
}
