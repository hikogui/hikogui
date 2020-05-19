// Copyright 2020 Pokitec
// All rights reserved.

#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace TTauri {

template<typename T>
class observer;

template<typename T>
class observed {
public:
    using value_type = T;

private:
    mutable std::mutex mutex;

    std::atomic<value_type> value;

    std::vector<observer<T> *> observers;

public:
    observed() noexcept {}
    observed(observed const &) = delete;
    observed(observed &&) = delete;
    observed &operator=(observed const &) = delete;
    observed &operator=(observed &&) = delete;

    observed(value_type rhs) noexcept : value(std::move(rhs)) {}

    operator value_type() const noexcept {
        return value.load(std::memory_order::memory_order_relaxed);
    }

    observed &operator=(value_type const &rhs) noexcept {
        value.store(rhs, std::memory_order::memory_order_relaxed);
        notify_observers(rhs);
        return *this;
    }

    observed &operator++() noexcept {
        auto new_value = value.fetch_add(1, std::memory_order::memory_order_relaxed);
        notify_observers(new_value + 1);
        return *this;
    }

    void register_observer(observer<T> *observer) noexcept {
        auto lock = std::scoped_lock(mutex);
        observers.push_back(observer);
        notify_observer(observer, value);
    }

    void unregister_observer(observer<T> *observer) noexcept {
        auto lock = std::scoped_lock(mutex);
        auto new_end = std::remove(observers.begin(), observers.end(), observer);
        observers.erase(new_end, observers.end());
    }

private:
    static void notify_observer(observer<T> *observer, value_type const &rhs) noexcept;

    void notify_observers(value_type const &rhs) const noexcept {
        auto lock = std::scoped_lock(mutex);
        for (let observer: observers) {
            notify_observer(observer, rhs);
        }
    }
};

}
