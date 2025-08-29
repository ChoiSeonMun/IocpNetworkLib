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
        static expected<ObjectPool, error_code> Create(size_t poolSize, ObjectFactory objectFactory, ResetAction resetAction) noexcept
        {
            try
            {
                Container container;
                container.reserve(poolSize);

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
                    container.push_back(lambdaCtor());
                }

                Pool pool;
                for (auto& obj : container)
                {
                    pool.push(&obj);
                }

                return ObjectPool(std::move(resetAction), std::move(container), std::move(pool));
            }
            catch (const std::bad_alloc&)
            {
                return std::unexpected(LibError::MemoryAllocationFailed);
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
            
            if (_resetAction)
            {
                _resetAction(obj);
            }
            
            auto deleter = [this](T* p) { Push(p); };

            return PooledObject(obj, deleter);
        }
    private:
        ObjectPool(ResetAction&& resetAction, Container&& container, Pool&& pool) noexcept
            :
            _resetAction(std::move(resetAction)),
            _container(std::move(container)),
            _pool(std::move(pool))
        {
        }

        void Push(T* object) noexcept
        {
            _pool.push(object);
        }
    private:
        ResetAction _resetAction;
        Container _container;
        Pool _pool;
    };
}