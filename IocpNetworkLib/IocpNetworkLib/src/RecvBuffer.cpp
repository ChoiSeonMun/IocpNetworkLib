#include "pch.h"
#include "csmnet/detail/RecvBuffer.h"

#include <span>

using namespace std;
namespace csmnet::detail
{
    expected<void, error_code> RecvBuffer::Resize(const size_t newSize) noexcept
    {
        try
        {
            _buffer.resize(newSize);
        }
        catch (const std::bad_alloc&)
        {
            return unexpected(LibError::MemoryAllocationFailed);
        }

        return {};
    }

    std::span<std::byte> RecvBuffer::GetWritableSpan(const size_t wantedSize) noexcept
    {
        const size_t freeSize = GetWritableSize();
        if (freeSize < wantedSize)
        {
            const size_t usedSize = _writePos - _readPos;
            if (usedSize == 0)
            {
                Reset();
            }
            else
            {
                std::memmove(_buffer.data(), _buffer.data() + _readPos, usedSize);
                _readPos = 0;
                _writePos = usedSize;
            }
        }

        return span(&_buffer[_writePos], wantedSize);
    }
}
