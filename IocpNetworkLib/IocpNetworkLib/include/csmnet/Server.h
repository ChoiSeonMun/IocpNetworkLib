#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/IocpCore.h"
#include "csmnet/detail/IServerApiForAcceptor.h"
#include "csmnet/detail/Acceptor.h"

#include "csmnet/util/ObjectPool.h"
#include "csmnet/util/ILogger.h"

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

namespace csmnet
{
    struct ServerConfig
    {
        uint16 Port;
        uint32 MaxSessionCount;
        uint32 IoThreadCount;
        uint32 SessionCleanIntervalSec;
    };

    template <std::derived_from<ServerSession> TSession>
    class Server : public detail::IServerApiForAcceptor
    {
    public:
        using SessionPool = util::ObjectPool<TSession>;
        using SessionFactory = SessionPool::ObjectFactory;
        using SessionResetAction = SessionPool::ResetAction;

        Server(util::ILogger& logger, ServerConfig config, SessionFactory sessionFactory, SessionResetAction sessionResetAction)
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

        expected<void, error_code> Open() noexcept
        {
            return _iocpCore.Open()
                .and_then([this]()
                    {
                        _acceptor = std::make_unique<detail::Acceptor>(_logger, _iocpCore, *this);
                        return _acceptor->Open(Endpoint::Any(_config.Port));
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

        void Close() noexcept
        {
            _isOpen = false;

            // TODO: 종료 신호 보내고 join 처리
            _iocpCore.Close();

            for (auto& thread : _ioThreads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
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
                auto result = _iocpCore.GetQueuedCompletionEvent()
                    .and_then([this](detail::IocpEvent* event) -> expected<void, error_code>
                        {
                            if (event)
                            {
                                event->Process();
                            }

                            return {};
                        });

                if (_isOpen == false)
                {
                    _logger.Info("Server::ProcessIO - Server Closed. Stop IO Thread.");
                    break;
                }

                if (!result)
                {
                    auto& error = result.error();
                    _logger.Error(format("Server::ProcessIO - {}: {}", error.value(), error.message()));
                }
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
        ServerConfig _config;
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