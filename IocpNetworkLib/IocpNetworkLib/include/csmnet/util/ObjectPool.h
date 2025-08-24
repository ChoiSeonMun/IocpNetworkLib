#pragma once

#include "csmnet/detail/Config.h"

#include <vector>
#include <stack>
#include <ranges>

namespace csmnet::util
{
    template <typename T>
    class PooledObject
    {
    public:
        PooledObject() noexcept = default;
        PooledObject(T* obj, class ObjectPool<T>* pool) noexcept
            : _object(obj), _pool(pool) {
        }
        PooledObject(const PooledObject&) = delete;
        PooledObject& operator=(const PooledObject&) = delete;
        PooledObject(PooledObject&& other) noexcept
            : _object(std::exchange(other._object, nullptr)),
            _pool(std::exchange(other._pool, nullptr))
        {
        }
        PooledObject& operator=(PooledObject&& other) noexcept
        {
            if (this != &other)
            {
                Reset();
                _object = std::exchange(other._object, nullptr);
                _pool = std::exchange(other._pool, nullptr);
            }
            return *this;
        }
        ~PooledObject() noexcept
        {
            Reset();
        }


        T* operator->() const noexcept { return _object; }
        T& operator*() const noexcept { return *_object; }
        T* Get() const noexcept { return _object; }

        explicit operator bool() const noexcept { return _object != nullptr; }
        bool IsValid() const noexcept { return _object != nullptr; }

        void Reset() noexcept
        {
            if (_object && _pool)
            {
                _pool->Push(_object);
                _object = nullptr;
                _pool = nullptr;
            }
        }
    private:
        T* _object = nullptr;
        class ObjectPool<T>* _pool = nullptr;
    };

    template <typename T, typename Container = std::vector<T>>
    class ObjectPool
    {
        friend class PooledObject<T>;
    public:
        using ResetAction = std::function<void(T*)>;
    public:
        ObjectPool(size_t poolSize, ResetAction resetAction)
            : _container(poolSize), _resetAction(resetAction)
        {
            _pool.push_range(_container);
        }
        ObjectPool(const ObjectPool&) = delete;
        ObjectPool& operator=(const ObjectPool&) = delete;
        ObjectPool(ObjectPool&&) noexcept = default;
        ObjectPool& operator=(ObjectPool&&) noexcept = default;
        ~ObjectPool() noexcept = default;

        PooledObject<T> Pop() noexcept
        {
            if (_pool.empty())
            {
                return {};
            }

            T* obj = _pool.top();
            _pool.pop();
            _resetAction(obj);
            
            return PooledObject<T>(obj, this);
        }
    private:
        void Push(T* object)
        {
            _pool.push(object);
        }
    private:
        ResetAction _resetAction;
        Container _container;
        std::stack<T*> _pool;
    };
}