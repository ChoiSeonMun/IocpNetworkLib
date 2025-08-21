#pragma once

#include "csmnet/detail/Config.h"
#include "csmnet/detail/IocpCore.h"

#include <memory>
#include <thread>
#include <atomic>
#include <vector>

namespace csmnet
{
    namespace detail { class IocpCore; class Acceptor; }

    class Server
    {
    public:
        Server() noexcept;
        virtual ~Server() noexcept;
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&&) noexcept = default;
        Server& operator=(Server&&) noexcept = default;
    
        expected<void, error_code> Open(uint32 maxSessionCount, uint16 port) noexcept;
        void Close() noexcept;

        bool IsOpen() const noexcept { return _isOpen.load(std::memory_order_acquire); }
    private:
        void ProcessIO();
    private:
        std::atomic<bool> _isOpen{ false };
        detail::IocpCore _iocpCore;
        std::unique_ptr<detail::Acceptor> _acceptor;
        std::vector<std::thread> _ioThreads;
    };
}