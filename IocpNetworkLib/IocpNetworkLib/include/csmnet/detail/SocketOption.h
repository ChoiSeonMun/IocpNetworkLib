#pragma once

#include "Config.h"

namespace csmnet::detail
{
    template <int Level, int Name>
    class Boolean final
    {
    public:
        Boolean() noexcept = default;
        explicit Boolean(bool value) noexcept : _value(value ? 1 : 0) {}
        Boolean& operator=(bool value)
        {
            _value = value ? 1 : 0;
            return *this;
        }

        int GetLevel() const noexcept { return Level; }
        int GetName() const noexcept { return Name; }
        void* GetData() noexcept { return &_value; }
        const void* GetData() const noexcept { return &_value; }
        size_t GetSize() const noexcept { return sizeof(_value); }
    private:
        int _value = 0;
    };

    template <int Level, int Name>
    class Linger final
    {
    public:
        Linger() noexcept = default;
        Linger(bool isOn, int timeout) noexcept
            : _linger( isOn ? 1 : 0, timeout )
        {
        }

        int GetLevel() const noexcept { return Level; }
        int GetName() const noexcept { return Name; }
        void* GetData() noexcept { return &_linger; }
        const void* GetData() const noexcept { return &_linger; }
        size_t GetSize() const noexcept { return sizeof(_linger); }
    private:
        linger _linger{ 0, 0 };
    };

    template <int Level, int Name>
    class UpdateAcceptContext final
    {
    public:
        UpdateAcceptContext() noexcept = default;
        explicit UpdateAcceptContext(SOCKET listenSocket) noexcept
            : _listenSocket(listenSocket)
        {
        }

        int GetLevel() const noexcept { return Level; }
        int GetName() const noexcept { return Name; }
        void* GetData() noexcept { return &_listenSocket; }
        const void* GetData() const noexcept { return &_listenSocket; }
        size_t GetSize() const noexcept { return sizeof(_listenSocket); }
    private:
        SOCKET _listenSocket = INVALID_SOCKET;
    };
}