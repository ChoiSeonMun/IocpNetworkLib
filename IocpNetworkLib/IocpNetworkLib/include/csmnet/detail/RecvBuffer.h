#pragma once

#include "Config.h"

#include <vector>
#include <span>

namespace csmnet::detail
{
    class RecvBuffer final
    {
    public:
        RecvBuffer() noexcept = default;
        ~RecvBuffer() noexcept = default;

        expected<void, error_code> Resize(size_t newSize) noexcept;

        void Reset() noexcept
        {
            _readPos = 0;
            _writePos = 0;
        }

        std::span<std::byte> GetWritableSpan() noexcept
        {
            const size_t freeSize = GetWritableSize();
            return GetWritableSpan(freeSize);
        }

        std::span<std::byte> GetWritableSpan(size_t wantedSize) noexcept;
        
        void CommitRead(const size_t size) noexcept
        {
            _readPos = (_readPos + size);
        }

        size_t GetWritableSize() const noexcept { return _buffer.capacity() - _writePos; }
    private:
        std::vector<std::byte> _buffer;
        size_t _readPos = 0;
        size_t _writePos = 0;
    };
}
