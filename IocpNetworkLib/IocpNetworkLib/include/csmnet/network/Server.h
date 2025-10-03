#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/detail/IServerApiForAcceptor.h"
#include "csmnet/detail/Acceptor.h"
#include "csmnet/util/ObjectPool.h"
#include "csmnet/util/ILogger.h"
#include "csmnet/dto/ServerConfig.h"


#include "ServerSession.h"

#include <unordered_set>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <stack>
#include <concepts>
#include <print>
#include <format>

namespace csmnet::network
{
    template <std::derived_from<ServerSession> TSession>
    class Server : public detail::IServerApiForAcceptor
    {
    public:
        using SessionPool = util::ObjectPool<TSession>;
        using SessionFactory = SessionPool::ObjectFactory;
        using SessionResetAction = SessionPool::ResetAction;

        Server(util::ILogger& logger, csmnet::dto::ServerConfig config, SessionFactory sessionFactory, SessionResetAction sessionResetAction)
            : _logger(logger)
            , _config(std::move(config))
            , _sessionPool(_config.MaxSessionCount, std::move(sessionFactory), std::move(sessionResetAction))
        {

        }

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&& other) = delete;
        Server& operator=(Server&& other) = delete;
        virtual ~Server() noexcept
        {
            Close();
        }

        virtual expected<void, error_code> Open() noexcept
        {
            return _iocpCore.Open()
                .and_then([this]()
                    {
                        _acceptor = std::make_unique<detail::Acceptor>(_logger, _iocpCore, *this);
                        return _acceptor->Open(csmnet::detail::Endpoint::Any(_config.Port));
                    })
                .and_then([this]() -> expected<void, error_code>
                    {
                        _isOpen = true;

                        for (size_t i = 0; i < _config.IoThreadCount; ++i)
                        {
                            _ioThreads.emplace_back(&Server::ProcessIO, this);
                        }

                        _sessionCleanerThread = std::jthread(&Server::CleanSessions, this);

                        return {};
                    });
        }

        virtual void Close() noexcept
        {
            if (bool wasOpened = _isOpen.exchange(false); wasOpened == false)
            {
                return;
            }

            _logger.Info("Server::Close - Shutting down...");

            const int32 ioThreadCount = static_cast<int32>(_ioThreads.size());
            for (int32 i = 0; i < ioThreadCount; ++i)
            {
                if (auto result = _iocpCore.Shutdown(); !result)
                {
                    _logger.Error(format("Shutdown Failed - {}: {}",
                        result.error().value(),
                        result.error().message()));
                }
            }
            
            for (auto& thread : _ioThreads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }

            _iocpCore.Close();
        }

        bool IsOpen() const noexcept { return _isOpen.load(std::memory_order_acquire); }

    private:
        // Implement IServerApiForAcceptor
        util::PooledObject<ServerSession> GetSession() noexcept override { return _sessionPool.Pop(); }
        void AddSession(const util::PooledObject<ServerSession>& session) noexcept override { _activeSessions.insert(session); }
    private:
        void ProcessIO()
        {
            while (_isOpen)
            {
                auto result = _iocpCore.GetQueuedCompletionEvent();
                if (!result)
                {
                    _logger.Error(format("Server::ProcessIO - {}: {}",
                        result.error().value(),
                        result.error().message()));
                    continue;
                }
                
                csmnet::detail::IocpEvent* event = *result;
                if (event == nullptr)
                {
                    _logger.Info("Server::ProcessIO - Received Shutdown Signal. Stop IO Thread.");
                    break;
                }

                event->Process();
            }
        }

        void CleanSessions()
        {
            while (_isOpen)
            {
                std::this_thread::sleep_for(std::chrono::seconds(_config.SessionCleanIntervalSec));

                _logger.Info(std::format("Server::CleanSessions - Current session : {}", _activeSessions.size()));
                erase_if(_activeSessions, [](const auto& session) { return session->IsConnected() == false; });
                _logger.Info(std::format("Server::CleanSessions - After Cleaning : {}", _activeSessions.size()));
            }
        }
    private:
        csmnet::dto::ServerConfig _config;
        util::ILogger& _logger;
        
        std::atomic<bool> _isOpen{ false };
        detail::IocpCore _iocpCore;
        std::unique_ptr<detail::Acceptor> _acceptor;

        util::ObjectPool<TSession> _sessionPool;
        std::unordered_set<util::PooledObject<ServerSession>> _activeSessions;

        std::vector<std::jthread> _ioThreads;
        std::jthread _sessionCleanerThread;
    };
}