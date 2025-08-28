#pragma once

#include "csmnet/detail/Config.h"

#include <vector>
#include <stack>
#include <ranges>

namespace csmnet::util
{
    template <typename T, typename Container = std::vector<T>>
    class ObjectPool
    {
    public:
        using ResetAction = std::function<void(T*)>;
    public:
        ObjectPool(size_t poolSize, ResetAction resetAction)
            :_resetAction(resetAction),
            _container(poolSize)
        {
            _pool.push_range(_container);
        }
        ObjectPool(const ObjectPool&) = delete;
        ObjectPool& operator=(const ObjectPool&) = delete;
        ObjectPool(ObjectPool&&) noexcept = default;
        ObjectPool& operator=(ObjectPool&&) noexcept = default;
        ~ObjectPool() noexcept = default;

        // 풀이 비었다면 nullptr일 수 있다.
        T* Pop() noexcept
        {
            if (_pool.empty())
            {
                return nullptr;
            }

            T* obj = _pool.top();
            _pool.pop();
            _resetAction(obj);
            
            return obj;
        }

        void Push(T* object) noexcept
        {
            _pool.push(object);
        }
    private:
        ResetAction _resetAction;
        Container _container;
        std::stack<T*, std::vector<T>> _pool;
    };
}