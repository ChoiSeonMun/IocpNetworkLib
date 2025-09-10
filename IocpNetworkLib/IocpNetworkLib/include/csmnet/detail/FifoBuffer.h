#pragma once

#include "Config.h"

#include <vector>
#include <span>

namespace csmnet::detail
{
    
    class FifoBuffer final
    {
    public:
        FifoBuffer() noexcept = default;
        ~FifoBuffer() noexcept = default;

        // std::bad_alloc이 발생할 수 있다.
        void Resize(size_t newSize)
        {
            CSM_ASSERT(newSize > 0);

            _buffer.resize(newSize);
        }

        void Reset() noexcept
        {
            _readPos = 0;
            _writePos = 0;
        }

        std::span<std::byte> GetWritableSpan() noexcept
        {
            const size_t writableSize = _buffer.capacity() - _writePos;
            CSM_ASSERT(writableSize > 0);

            return { &_buffer[_writePos], writableSize };
        }

        std::span<const std::byte> GetReadableSpan(size_t readSize) noexcept
        {
            CSM_ASSERT(readSize > 0);

            _writePos += readSize;
            const size_t readableSize = _writePos - _readPos;

            return { &_buffer[_readPos], readableSize};
        }

        void CommitRead(size_t size) noexcept
        {
            CSM_ASSERT(size > 0);

            _readPos += size;
            if (_readPos == _writePos)
            {
                Reset();
            }
        }
    private:
        std::vector<std::byte> _buffer;
        size_t _readPos = 0;
        size_t _writePos = 0;
    };
}
