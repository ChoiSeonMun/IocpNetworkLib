#include "pch.h"
#include "csmnet/Client.h"
#include "csmnet/Endpoint.h"

#include <print>

using namespace std;

namespace csmnet
{
    using namespace detail;

    Client::~Client() noexcept
    {
        Close();
    }

    expected<void, error_code> Client::Connect(const Endpoint& serverEndpoint) noexcept
    {
        return _socket.Open(SocketType::Tcp)
            .and_then([this]() -> expected<void, error_code>
                {
                    return _iocpCore.Open();
                })
            .and_then([this, &serverEndpoint]() -> expected<void, error_code>
                {
                    _iocpCore.Register(_socket);
                    
                    return _socket.Bind(Endpoint::Any(0));
                })
            .and_then([this, &serverEndpoint]()
                {
                    return  _socket.ConnectEx(serverEndpoint, _connectEvent);
                })
            .and_then([this]() -> expected<void, error_code>
                {
                    _isConnected = true;
                    _ioThread = std::thread(&Client::ProcessIO, this);
                    return {};
                });
    }

    void Client::Close() noexcept
    {
        if (bool wasConnected = _isConnected.exchange(false); wasConnected == false)
        {
            return;
        }

        _socket.Close();
        
        if (_ioThread.joinable())
        {
            _ioThread.join();
        }

        _iocpCore.Close();
    }

    void Client::Process(detail::AcceptEvent* event)
    {
        // This should not be called for a client
    }

    void Client::Process(detail::ConnectEvent* event)
    {
        std::println("Connection established!");
    }

    void Client::Process(detail::RecvEvent* event)
    {
        // TODO: Implement
    }

    void Client::Process(detail::SendEvent* event)
    {
        // TODO: Implement
    }

    void Client::ProcessIO()
    {
        while (_isConnected)
        {
            auto result = _iocpCore.GetQueuedCompletionEvent()
                .and_then([](IocpEvent* event) -> expected<void, error_code>
                    {
                        if (event)
                        {
                            event->Process();
                        }

                        return {};
                    });

            if (_isConnected == false)
            {
                std::println("Client disconnected, stopping IO processing.");
                break;
            }
            
            if (!result)
            {
                auto error = result.error();
                std::println("[Error {}] Client::ProcessIO - {}", error.value(), error.message());
            }
        }
    }
}