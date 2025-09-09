#include "pch.h"
#include "csmnet/detail/Session.h"
#include "csmnet/util/ILogger.h"

using namespace std;
using namespace csmnet::detail;

namespace csmnet
{
    Session::Session(util::ILogger& logger) noexcept
        : _logger(logger)
    {
    }

    Session::Session(Session&& other) noexcept
        : _isConnected(other._isConnected.load(std::memory_order_acquire))
        , _socket(std::move(other._socket))
        , _remote(std::move(other._remote))
        , _logger(other._logger)
        , _sendEvent(*this)
        , _recvEvent(*this)
        , _recvBuffer(std::move(other._recvBuffer))
    {
        other._isConnected.store(false, std::memory_order_release);
    }

    Session& Session::operator=(Session&& other) noexcept
    {
        if (this != &other)
        {
            _isConnected.store(other._isConnected.load(std::memory_order_acquire), std::memory_order_release);
            _socket = std::move(other._socket);
            _remote = std::move(other._remote);
            _recvBuffer = std::move(other._recvBuffer);
            other._isConnected.store(false, std::memory_order_release);
        }
        return *this;
    }

    expected<void, error_code> Session::Send(std::span<const std::byte> message) noexcept
    {
        if (_isConnected == false)
        {
            return unexpected(LibError::SessionClosed);
        }

        return PostSend(message);
    }

    void Session::Process(RecvEvent* event)
    {
        CSM_ASSERT(event != nullptr);

        if (event->IsRemoteClosed())
        {
            OnRemoteClosed();
            return;
        }

        const auto message = _recvBuffer.GetReadableSpan(event->GetBytesTransferred());
        OnRecv(message);
        _recvBuffer.CommitRead(message.size());

        if (auto result = PostRecv(); !result)
        {
            _logger.Error(format("Session::Process(RecvEvent) - fail to post recv : [{}] {}",
                result.error().value(),
                result.error().message()));

            Disconnect();
            return;
        }
    }

    void Session::Process(SendEvent* event)
    {
        CSM_ASSERT(event != nullptr);
    }

    expected<void, error_code> Session::PostRecv() noexcept
    {
        const auto buffer = _recvBuffer.GetWritableSpan();

        _recvEvent.Reset(buffer);
        return _socket.RecvEx(_recvEvent);
    }

    expected<void, error_code> Session::PostSend(std::span<const std::byte> message) noexcept
    {
        _sendEvent.Reset(message);
        return _socket.SendEx(_sendEvent);
    }

    expected<void, error_code> Session::Activate() noexcept
    {
        _isConnected = true;

        return PostRecv()
            .and_then([this]() -> expected<void, error_code>
                {
                    OnConnected();

                    return {};
                });
    }
}
