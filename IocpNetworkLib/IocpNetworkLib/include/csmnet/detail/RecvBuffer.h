#pragma once

#include "Config.h"

#include <vector>

namespace csmnet::detail
{
    class RecvBuffer final
    {
    public:
        explicit RecvBuffer(const size_t capacity);
        ~RecvBuffer() noexcept = default;

        void Reset() noexcept
        {
            _readPos = 0;
            _writePos = 0;
        }
        
        char* GetWriteBuffer(const size_t wantedSize);

        void CommitRead(const size_t size) noexcept
        {
            _readPos = (_readPos + size);
        }

        size_t GetFreeLinearSize() const noexcept
        {
            return GetCapacity() - _writePos;
        }

        size_t GetCapacity() const noexcept { return _buffer.capacity(); }
    private:
        std::vector<char> _buffer;
        size_t _readPos;
        size_t _writePos;
    };
}
