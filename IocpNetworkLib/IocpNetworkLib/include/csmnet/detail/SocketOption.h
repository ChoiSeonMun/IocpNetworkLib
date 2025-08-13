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

        int level() const noexcept { return Level; }
        int name() const noexcept { return Name; }
        void* data() noexcept { return &_value; }
        const void* data() const noexcept { return &_value; }
        size_t size() const noexcept { return sizeof(_value); }
    private:
        int _value = 0;
    };

    template <int Level, int Name>
    class Linger final
    {
    public:
        Linger() noexcept = default;
        Linger(bool isOn, int timeout) noexcept
            : _linger{ isOn ? 1 : 0, timeout }
        {
        }

        int level() const noexcept { return Level; }
        int name() const noexcept { return Name; }
        void* data() noexcept { return &_linger; }
        const void* data() const noexcept { return &_linger; }
        size_t size() const noexcept { return sizeof(_linger); }
    private:
        linger _linger{ 0, 0 };
    };
}