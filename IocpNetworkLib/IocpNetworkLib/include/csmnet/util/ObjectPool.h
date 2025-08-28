#pragma once

#include "csmnet/detail/Config.h"

#include <vector>
#include <stack>
#include <ranges>

namespace csmnet::util
{
    template <typename T>
    using PooledObject = std::shared_ptr<T>;

    template <typename T, typename Container = std::vector<T>>
    class ObjectPool
    {
    public:
        using PooledObject = util::PooledObject<T>;
        using ResetAction = std::function<void(T*)>;
    public:
        ObjectPool(size_t poolSize, ResetAction resetAction)
            :_resetAction(resetAction),
            _container(poolSize)
        {
            for (auto& obj : _container)
            {
                _pool.push(&obj);
            }
        }
        ObjectPool(const ObjectPool&) = delete;
        ObjectPool& operator=(const ObjectPool&) = delete;
        ObjectPool(ObjectPool&&) noexcept = default;
        ObjectPool& operator=(ObjectPool&&) noexcept = default;
        ~ObjectPool() noexcept = default;

        // 풀이 비었다면 nullptr일 수 있다.
        PooledObject Pop() noexcept
        {
            if (_pool.empty())
            {
                return nullptr;
            }

            T* obj = _pool.top();
            _pool.pop();
            _resetAction(obj);

            auto deleter = [this](T* p) { Push(p); };

            return PooledObject(obj, deleter);
        }
    private:
        void Push(T* object) noexcept
        {
            _pool.push(object);
        }
    private:
        ResetAction _resetAction;
        Container _container;
        std::stack<T*, std::vector<T*>> _pool;
    };
}