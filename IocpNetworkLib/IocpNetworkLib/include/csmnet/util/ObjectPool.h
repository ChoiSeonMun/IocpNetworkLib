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
        using Pool = std::stack<T*, std::vector<T*>>;
        using PooledObject = util::PooledObject<T>;
        using ObjectFactory = std::function<T()>;
        using ResetAction = std::function<void(T*)>;
    public:
        // std::bad_alloc 예외가 발생할 수 있다.
        ObjectPool(size_t poolSize, ObjectFactory objectFactory, ResetAction resetAction)
            : _resetAction(std::move(resetAction))
        {
            CSM_ASSERT(poolSize > 0);
            CSM_ASSERT(objectFactory != nullptr);

            _container.reserve(poolSize);

            std::function<T()> lambdaCtor{ };
            if (objectFactory)
            {
                lambdaCtor = std::move(objectFactory);
            }
            else
            {
                lambdaCtor = []() { return T(); };
            }

            for (size_t i = 0; i < poolSize; ++i)
            {
                _container.push_back(lambdaCtor());
            }

            for (auto& obj : _container)
            {
                _pool.push(&obj);
            }
        }
        ObjectPool(const ObjectPool&) = delete;
        ObjectPool& operator=(const ObjectPool&) = delete;
        ObjectPool(ObjectPool&&) noexcept = delete;
        ObjectPool& operator=(ObjectPool&&) noexcept = delete;
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
            
            if (_resetAction)
            {
                _resetAction(obj);
            }
            
            auto deleter = [this](T* p) { Push(p); };

            return PooledObject(obj, deleter);
        }
    private:
        void Push(T* object) noexcept
        {
            _pool.push(object);
        }
    private:
        ResetAction _resetAction{ nullptr };
        Container _container;
        Pool _pool;
    };
}