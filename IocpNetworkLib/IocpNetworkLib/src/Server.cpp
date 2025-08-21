#include "pch.h"
#include "csmnet/Server.h"
#include "csmnet/detail/Acceptor.h"

#include <print>

using namespace std;

namespace csmnet
{
    using namespace detail;

    Server::Server() noexcept
    {
    }

    Server::~Server() noexcept
    {
        Close();
    }

    expected<void, error_code> Server::Open(uint32 maxSessionCount, uint16 port) noexcept
    {
        return _iocpCore.Open()
            .and_then([this, maxSessionCount, port]()
                {
                    _acceptor = make_unique<Acceptor>(_iocpCore, maxSessionCount);
                    return _acceptor->Open(Endpoint::Any(port));
                })
            .and_then([this]() -> expected<void, error_code>
                {
                    _isOpen = true;

                    // IO 처리 스레드 시작
                    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
                    {
                        _ioThreads.emplace_back(&Server::ProcessIO, this);
                    }

                    return {};
                });
    }

    void Server::Close() noexcept
    {
        _isOpen = false;
    }

    void Server::ProcessIO()
    {
        while (_isOpen)
        {
            auto result = _iocpCore.GetQueuedCompletionEvent()
                .and_then([this](IocpEvent* event) -> expected<void, error_code>
                    {
                        if (event)
                        {
                            event->Process();
                        }

                        return {};
                    });

            if (!result)
            {
                println("Error Processing IO: {}", result.error().message());
            }
        }
    }
}