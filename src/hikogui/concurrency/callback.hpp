// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <memory>
#include <atomic>
#include <functional>
#include <cstddef>

hi_export_module(hikogui.concurrency.callback);

hi_export namespace hi { inline namespace v1 {
namespace detail {

template<typename R, typename... Args>
struct callback_base {
    virtual R operator()(Args... args) = 0;

    /** Check if the callback-function is expired.
     */
    [[nodiscard]] bool expired() const noexcept
    {
        return (count.load(std::memory_order::relaxed) & 1) == 0;
    }

    /** Check if the callback-function is locked.
     */
    [[nodiscard]] bool locked() const noexcept
    {
        return count.load(std::memory_order::relaxed) >= 2;
    }

    /** Lock before calling the callback-function.
     *
     * This will delay the destruction of the callback object and in turn
     * delay the destruction of the object that stores the callback function.
     *
     * @retval true The callback-function may be called
     * @retval false The callback-function has been destroyed.
     */
    [[nodiscard]] bool lock() const noexcept
    {
        auto expected = count.load(std::memory_order::relaxed);
        do {
            hi_axiom(expected < std::numeric_limits<std::atomic_unsigned_lock_free::value_type>::max() - 2);

            if ((expected & 1) == 0) {
                // The callback object is being/was destructed.
                return false;
            }
        } while (not count.compare_exchange_strong(expected, expected + 2, std::memory_order::relaxed));

        return true;
    }

    /** Unlock after calling the callback-function
     */
    void unlock() const noexcept
    {
        if (count.fetch_sub(2, std::memory_order::relaxed) == 2) {
            // Last inflight callback has finished,
            // and callback object is being destroyed.
            // Notify a callback object that is delaying destruction.
            count.notify_one();
        }
    }

    /** Destruct the callback objects without delaying.
     */
    void unsafe_destruct() const noexcept
    {
        hilet prev_count = count.fetch_sub(1, std::memory_order::relaxed);
        hi_axiom((prev_count & 1) == 1);
    }

    /** Delay the destruction of the callback object, when calls are in flight.
     */
    void delay_destruct() const noexcept
    {
        if (auto expected = count.fetch_sub(1, std::memory_order::relaxed); expected != 0) {
            // There are callbacks in-flight, wait for any callbacks to finish.
            do {
                count.wait(expected, std::memory_order::relaxed);
                expected = count.load(std::memory_order::relaxed);

                hi_axiom((expected & 1) == 0);
            } while (expected != 0);
        }
    }

    /** Reference count for callbacks.
     *
     * Bit meaning:
     *  - [0] A `callback`-object exists.
     *  - [:1] Number of outstanding calls.
     */
    mutable std::atomic_unsigned_lock_free count = 1;
};

template<typename Function, typename R, typename... Args>
struct callback_impl : callback_base<R, Args...> {
    template<typename Func>
    callback_impl(Func&& func) : callback_base<R, Args...>(), func(std::forward<Func>(func))
    {
    }

    /** Call the callback function.
     *
     * @param args The arguments forwarded to callback-function.
     * @return The result of the callback-function.
     * @throws The exception thrown from the callback-function.
     */
    R operator()(Args... args) override
    {
        return func(std::forward<Args>(args)...);
    }

    Function func;
};

} // namespace detail

template<typename T = void()>
class weak_callback;

template<typename T = void()>
class callback;

template<typename R, typename... Args>
class callback<R(Args...)>;

template<typename R, typename... Args>
class weak_callback<R(Args...)> {
public:
    using result_type = R;
    using base_impl_type = detail::callback_base<R, Args...>;

    constexpr weak_callback() noexcept = default;
    weak_callback(weak_callback const& other) noexcept = default;
    weak_callback(weak_callback&& other) noexcept = default;
    weak_callback& operator=(weak_callback const& other) noexcept = default;
    weak_callback& operator=(weak_callback&& other) noexcept = default;

    weak_callback(std::shared_ptr<base_impl_type> other) noexcept : _impl(std::move(other)) {}
    weak_callback(callback<R(Args...)> const& other) noexcept;

    /** Check if the callback object is expired.
     *
     * @retval false The callback object is functioning.
     * @retval true The callback object is destroyed or in the process of being
     *         destroyed.
     */
    [[nodiscard]] bool expired() const noexcept
    {
        if (_impl) {
            return _impl->expired();
        } else {
            return true;
        }
    }

    /** Make the weak_callback expired, so that it can no longer be called.
     */
    void reset() noexcept
    {
        _impl.reset();
    }

    /** Lock before calling the callback-function.
     *
     * This will delay the destruction of the callback object and in turn
     * delay the destruction of the object that stores the callback function.
     *
     * @retval true The callback-function may be called
     * @retval false The callback-function has been destroyed.
     */
    [[nodiscard]] bool lock() const noexcept
    {
        if (_impl) {
            return _impl->lock();
        } else {
            return false;
        }
    }

    /** Unlock after calling the callback-function
     */
    void unlock() const noexcept
    {
        hi_axiom_not_null(_impl);
        return _impl->unlock();
    }

    /** Call the callback function.
     *
     * This may delay the destruction of the callback object and the object
     * that stores the callback object.
     *
     * @pre This object must first be locked.
     * @param args The arguments to forward to the function.
     * @return The result value of the function
     * @throws std::bad_function_call if *this does not store a callable function target.
     * @throws Any exception thrown from the callback function.
     */
    result_type operator()(Args... args) const
    {
        if (not _impl) {
            throw std::bad_function_call();
        }

        hi_axiom(_impl->locked(), "A weak_callback must be locked before calling the function operator.");
        return (*_impl)(std::forward<Args>(args)...);
    }

private:
    std::shared_ptr<base_impl_type> _impl;
};

/** A callback function.
 *
 * This callback object holds a function object that can be called.
 * It works mostly as `std::move_only_function<R(Args...)>`.
 *
 * When a object subscribes a callback function it will take a
 * `weak_callback<R(Args...)>` of this callback object.
 *
 * This callback type is specifically designed to more safely handle capturing
 * a `this` pointer into a lambda.
 *
 * The idea is that a `subscribe()` function will return a callback object
 * that will be stored inside the object for which the `this` pointer is
 * captured. This means that if this object is destroyed, the callback is
 * destroyed as well, and the caller will notice this through the
 * `weak_callback`.
 *
 * @tparam R The result of the function.
 * @tparam Args The arguments of the function.
 */
template<typename R, typename... Args>
class callback<R(Args...)> {
public:
    using result_type = R;
    using weak_type = weak_callback<R(Args...)>;

    using base_impl_type = detail::callback_base<R, Args...>;
    template<typename Func>
    using impl_type = detail::callback_impl<Func, R, Args...>;

    constexpr callback() = default;
    callback(callback const&) = delete;
    callback& operator=(callback const&) = delete;

    /** Destroy the callback.
     * 
     * This may be delayed if there are calls to the callback function inflight
     * on another thread.
     */
    ~callback()
    {
        unsubscribe();
    }

    callback(callback&& other) noexcept : _impl(std::exchange(other._impl, nullptr)) {}

    callback& operator=(callback&& other) noexcept
    {
        unsubscribe();
        _impl = std::exchange(other._impl, nullptr);
        return *this;
    }

    callback(std::nullptr_t) noexcept : _impl(nullptr) {}

    callback& operator=(std::nullptr_t) noexcept
    {
        unsubscribe();
        return *this;
    }

    template<typename Func>
    explicit callback(Func&& func) : _impl(std::make_shared<impl_type<Func>>(std::forward<Func>(func)))
    {
    }

    /** Unsafe unsubscribe the callback.
     * 
     * This function can be called from within the current inflight call,
     * to be able to destroy the owner of the callback and the callback itself
     * from within the executing callback.
     * 
     * The unsubscribe is lazy, although the callback can no longer be called,
     * the actual deallocation of the callback may happen at a later timer.
     * 
     * @return A shared_ptr of the actual function implementation. You should
     *         capture this so that the frame of the lambda remains available
     *         until it terminates.
     */
    [[nodiscard]] std::shared_ptr<base_impl_type> unsafe_unsubscribe() noexcept
    {
        if (_impl) {
            _impl->unsafe_destruct();
        }
        return std::exchange(_impl, nullptr);
    }

    /** Unsubscribe the callback.
     * 
     * Unsubscribe will block until there are no more inflight calls to this
     * callback.
     * 
     * The unsubscribe is lazy, although the callback can no longer be called,
     * the actual deallocation of the callback may happen at a later timer.
     */
    void unsubscribe() noexcept
    {
        if (_impl) {
            _impl->delay_destruct();
        }
        _impl = nullptr;
    }

    [[nodiscard]] operator bool() const noexcept
    {
        return static_cast<bool>(_impl);
    }

    /** Call the callback function.
     *
     * @param args The arguments to forward to the function.
     * @return The result value of the function
     * @throws std::bad_function_call if *this does not store a callable function target.
     * @throws Any exception thrown from the callback function.
     */
    R operator()(Args... args) const
    {
        if (not _impl) {
            throw std::bad_function_call();
        }

        // The callback function does not need to be locked as the callback
        // object can not be destroyed at this point.
        return (*_impl)(std::forward<Args>(args)...);
    }

private:
    std::shared_ptr<base_impl_type> _impl = {};

    callback(std::shared_ptr<base_impl_type>&& other) : _impl(std::move(other)) {}

    friend weak_callback<R(Args...)>;
};

template<typename R, typename... Args>
hi_inline weak_callback<R(Args...)>::weak_callback(callback<R(Args...)> const& other) noexcept : weak_callback(other._impl)
{
}

}} // namespace hi::v1
