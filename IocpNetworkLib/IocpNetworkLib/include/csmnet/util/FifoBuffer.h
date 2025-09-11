// -------------------------------------------------------
// FifoBuffer.h
// -------------------------------------------------------
// 
// Zero Copy를 지원하고자 만든 FIFO 방식의 버퍼
// 버퍼에 데이터를 쓰려면 Poke() 후 Advance()를 호출해야 한다.
// 버퍼로부터 데이터를 읽으려면 Peek() 후 Consume()를 호출해야 한다.

#pragma once

#include "csmnet/detail/Config.h"

#include <vector>
#include <span>

namespace csmnet::util
{
    template <typename T>
    class FifoBuffer final
    {
    public:
        FifoBuffer() noexcept = default;
        ~FifoBuffer() noexcept = default;

        // 버퍼 크기 재설정
        // 메모리 할당에 실패하면 std::bad_alloc 예외가 발생함
        void Resize(size_t newSize)
        {
            CSM_ASSERT(newSize > 0);

            _buffer.resize(newSize);
        }

        // 현재 쓸 수 있는 버퍼 내부 영역을 반환한다.
        // 데이터를 썼다면 Advance()를 호출하여 쓴 만큼 쓰기 포인터를 옮겨야 한다.
        std::span<T> Poke() noexcept
        {
            return Poke(GetWritableSize());
        }

        // 현재 쓸 수 있는 버퍼 내부 영역을 반환한다.
        // 반환된 span은 writeSize보다 작을 수 있다. 즉, 실제 쓸 수 있는만큼만 반환한다.
        // 데이터를 썼다면 Advance()를 호출하여 쓴 만큼 쓰기 포인터를 옮겨야 한다.
        std::span<T> Poke(size_t writeSize) noexcept
        {
            return { &_buffer[_writePos], std::min(GetWritableSize(), writeSize) };
        }

        // 현재 읽을 수 있는 버퍼 내부 영역을 반환한다.
        // 데이터를 읽었다면 Consume()를 호출하여 읽은 만큼 읽기 포인터를 옮겨야 한다.
        std::span<const T> Peek() noexcept
        {
            return Peek(GetReadableSize());
        }

        // readSize 만큼 읽을 수 있는 버퍼 내부 영역을 반환한다.
        // 반환된 span은 readSize보다 작을 수 있다. 즉, 실제 읽을 수 있는만큼만 반환한다.
        // 데이터를 읽었다면 Consume()를 호출하여 읽은 만큼 읽기 포인터를 옮겨야 한다.
        std::span<const T> Peek(size_t readSize) noexcept
        {
            return { &_buffer[_readPos], std::min(GetReadableSize(), readSize) };
        }

        // 쓰기 포인터의 위치를 옮긴다.
        // 이동 후의 쓰기 포인터의 위치가 유효하지 않다면 false를 반환하며 반영되지 않는다.
        bool Advance(size_t length) noexcept
        {
            const size_t newPos = _writePos + length;
            if (newPos >= _buffer.size())
            {
                return false;
            }

            _writePos = newPos;
            return true;
        }

        // 읽기 포인터의 위치를 옮긴다.
        // 이동 후의 읽기 포인터의 위치가 유효하지 않다면 false를 반환하며 반영되지 않는다.
        bool Consume(size_t length) noexcept
        {
            const size_t newPos = _readPos + length;
            if (newPos > _writePos)
            {
                return false;
            }

            _readPos = newPos;
            return true;
        }

        // wantedSize 만큼의 공간을 확보한다.
        // 확보되었다면 true, 그렇지 않다면 false를 반환한다.
        bool EnsureWritable(size_t wantedSize) noexcept
        {
            if (_buffer.empty())
            {
                return wantedSize == 0;
            }

            if (_readPos == _writePos)
            {
                _writePos = 0;
                _readPos = 0;
            }

            if (GetWritableSize() >= wantedSize)
            {
                return true;
            }

            const size_t readableSize = GetReadableSize();
            std::copy(&_buffer[_readPos], &_buffer[_writePos], &_buffer[0]);
            _writePos = readableSize;
            _readPos = 0;
            
            return GetWritableSize() >= wantedSize;
        }

        // 버퍼에 쓸 수 있는 크기를 반환한다.
        size_t GetWritableSize() const noexcept
        {
            return _buffer.size() - _writePos;
        }

        // 버퍼에서 읽을 수 있는 크기를 반환한다.
        size_t GetReadableSize() const noexcept
        {
            return _writePos - _readPos;
        }
    private:
        std::vector<T> _buffer;
        size_t _readPos = 0;
        size_t _writePos = 0;
    };
}
