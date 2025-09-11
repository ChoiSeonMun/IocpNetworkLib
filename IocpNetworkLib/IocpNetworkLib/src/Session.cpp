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
        , _sendBuffer(std::move(other._sendBuffer))
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
            _sendBuffer = std::move(other._sendBuffer);
            _recvBuffer = std::move(other._recvBuffer);
            other._isConnected.store(false, std::memory_order_release);
        }
        return *this;
    }

    expected<void, error_code> Session::Send(util::FifoBuffer<byte>& buffer) noexcept
    {
        if (_isConnected == false)
        {
            return unexpected(LibError::SessionClosed);
        }

        return PostSend(buffer);
    }

    void Session::Process(RecvEvent* event)
    {
        CSM_ASSERT(event != nullptr);

        if (event->IsRemoteClosed())
        {
            OnRemoteClosed();
            return;
        }

        _recvBuffer.Advance(event->GetRecvByte());
        const auto message = _recvBuffer.Peek(event->GetRecvByte());
        OnRecv(message);
        _recvBuffer.Consume(message.size());

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

        event->ClearSendBuffers();
        _isSending = false;
    }

    expected<void, error_code> Session::PostRecv() noexcept
    {
        bool ensureResult = _recvBuffer.EnsureWritable(kMinRecvSize);
        CSM_ASSERT(ensureResult == true);
        const auto buffer = _recvBuffer.Poke();
        
        _recvEvent.Reset(buffer);
        return _socket.RecvEx(_recvEvent);
    }

    expected<void, error_code> Session::PostSend(util::FifoBuffer<byte>& sendBuffer) noexcept
    {
        _sendEvent.Reset();
        _sendEvent.AddSendBuffer(&sendBuffer);

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
