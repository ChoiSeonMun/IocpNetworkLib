#include "pch.h"
#include "csmnet/detail/RecvBuffer.h"

namespace csmnet::detail
{
    RecvBuffer::RecvBuffer(const size_t capacity)
        : _buffer(capacity)
        , _readPos(0)
        , _writePos(0)
    {
    }

    char* RecvBuffer::GetWriteBuffer(const size_t wantedSize)
    {
        if (GetFreeLinearSize() < wantedSize)
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

        return &_buffer[_writePos];
    }

}
