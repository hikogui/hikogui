// Copyright 2020 Pokitec
// All rights reserved.

#pragma once
#include "TTauri/Foundation/observed.hpp"
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace TTauri {


template<typename T>
class observer {
public:
    using value_type = T;
    using observed_type = observed<T>;
    using callback_type = std::function<void(T const &)>;

    friend observed_type;
private:
    /** The internal copy of the observed value.
     * This will also allow to use the observer stand-alone.
     * The value may be updated by multiple threads at the same time.
     */
    std::atomic<value_type> value;

    /** A pointer to the object to be observed.
     * If the pointer is nullptr, then the observer is used stand-alone.
     */
    observed_type *observed_object;

    /** The callback to call when the observed object is updated.
     */
    callback_type callback;

    /** Update value and call callbacks.
     * This function is used by the observed when its value is updated.
     */
    template<typename Value>
    void update(Value &rhs) noexcept {
        value = std::forward<Value>(rhs);
        if (callback) {
            callback(value);
        }
    }

    /** Call the callback with the current value.
    */
    void do_callback(value_type const &value) const noexcept {
        if (callback) {
            callback(value);
        }
    }

public:
    observer(observer const &) = delete;
    observer(observer &&) = delete;
    observer &operator=(observer const &) = delete;
    observer &operator=(observer &&) = delete;

    ~observer() {
        if (observed_object) {
            observed_object->unregister_observer(this);
        }
    }

    observer(observed_type &value, callback_type callback={}) noexcept :
        observed_object(&value), callback(std::move(callback))
    {
        observed_object->register_observer(this);
    }

    template<typename Value>
    observer(Value &&value, callback_type callback={}) noexcept :
        observed_object(nullptr), callback(std::move(callback))
    {
        (*this) = std::forward<Value>(value);
    }

    observer(callback_type callback={}) noexcept :
        observed_object(nullptr), callback(std::move(callback))
    {
        (*this) = value_type{};
    }

    /** Return the cached or internal value.
     */
    operator value_type () const noexcept {
        return value.load(std::memory_order::memory_order_relaxed);
    }

    /** Register an observed object.
    * This function should be called shortly after construction of the observer,
    * as observer updates are non atomic.
    *
    * This will call the callback function immediatly to the value
    * of the observed value.
    */
    void register_observed(observed_type &observed) noexcept {
        observed_object = &observed;
        observed_object->register_observer(this);
    }

    void unregister_observed() noexcept {
        if (observed_object) {
            observed_object->unregister_observer(this);
        }
        observed_object = nullptr;
    }

    /** Register a callback function for when the value is updated.
     * This function should be called shortly after construction of the observer,
     * as callback updates are non atomic.
     *
     * This will call the callback function immediatly as from the
     * point of view of the callback function the value was updated
     * for the first time.
     */
    template<typename Callback>
    void register_callback(Callback &&callback) noexcept {
        callback = std::forward<Callback>(callback);
        if (callback) {
            callback(value);
        }
    }

    void unregister_callback() noexcept {
        callback = nullptr;
    }

    /** Update value.
     */
    observer &operator=(value_type const &rhs) noexcept {
        if (observed_object) {
            (*observed_object) = rhs;
        } else {
            value.store(rhs, std::memory_order::memory_order_relaxed);
            do_callback(rhs);
        }
        return *this;
    }

    observer &operator++() noexcept {
        if (observed_object) {
            ++(*observed_object);
        } else {
            auto new_value = value.fetch_add(1, std::memory_order::memory_order_relaxed);
            do_callback(new_value + 1);
        }
        return *this;
    }
};

template<typename T>
void observed<T>::notify_observer(observer<T> *observer, T const &rhs) noexcept
{
    ttauri_assume(observer != nullptr);
    observer->update(rhs);
}


}