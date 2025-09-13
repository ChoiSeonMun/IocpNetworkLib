#include "pch.h"
#include "csmnet/network/Client.h"
#include "csmnet/network/ClientSession.h"
#include "csmnet/detail/Endpoint.h"

#include <print>

using namespace std;
using namespace csmnet::detail;
using namespace csmnet::dto;

namespace csmnet::network
{
    Client::Client(ClientSessionFactory sessionFactory, ClientConfig config) noexcept
        : _sessionFactory(std::move(sessionFactory)),
        _config(std::move(config))
    {
    }

    Client::~Client() noexcept
    {

    }

    expected<void, error_code> Client::Run() noexcept
    {
        if (_sessionFactory == nullptr)
        {
            return unexpected(LibError::CannotCreateSession);
        }

        return _iocpCore.Open()
            .and_then([this]()
                {
                    return csmnet::detail::Endpoint::From(_config.ServerIp, _config.ServerPort);
                })
            .and_then([this](Endpoint&& serverEndpoint) -> expected<void, error_code>
                {
                    for (int32 i = 0; i < _config.SessionCount; ++i)
                    {
                        _sessions.push_back(_sessionFactory());
                    }

                    for (const auto& session : _sessions)
                    {
                        if (auto result = session->Connect(serverEndpoint, _iocpCore); !result)
                        {
                            return unexpected(result.error());
                        }
                    }

                    return {};
                })
            .and_then([this]() -> expected<void, error_code>
                {
                    _isRunning = true;
                    for (int32 i = 0; i < _config.IoThreadCount; ++i)
                    {
                        _ioThreads.emplace_back(&Client::DispatchIO, this);
                    }

                    return {};
                });
    }

    void Client::Stop() noexcept
    {
        if (bool wasRunning = _isRunning.exchange(false); wasRunning == false)
        {
            return;
        }

        for (const auto& session : _sessions)
        {
            session->Disconnect();
        }
        _isRunning = false;
    }

    void Client::DispatchIO()
    {
        while (IsRunning())
        {
            std::println("Processing...");
            auto result = _iocpCore.GetQueuedCompletionEvent()
                .and_then([](IocpEvent* event) -> expected<void, error_code>
                    {
                        if (event)
                        {
                            event->Process();
                        }

                        return {};
                    });

            if (IsRunning() == false)
            {
                std::println("Client::DispatchIO - Stop IO");
                break;
            }
            
            if (!result)
            {
                auto error = result.error();
                std::println("Client::ProcessIO - [Error] {}:{}", error.value(), error.message());
            }
        }
    }
}