// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "thread.hpp"
#include "../macros.hpp"
#include <memory>
#include <atomic>
#include <functional>
#include <cstddef>
#include <mutex>
#include <algorithm>

hi_export_module(hikogui.concurrency.callback);

hi_export namespace hi {
inline namespace v1 {
namespace detail {

template<typename ResultType, typename... ArgTypes>
class callback_base {
public:
    virtual ~callback_base() = default;

    virtual ResultType operator()(ArgTypes... args) = 0;
};

template<typename FunctionType, typename ResultType, typename... ArgTypes>
class callback_impl : public callback_base<ResultType, ArgTypes...> {
public:
    ~callback_impl()
    {
#ifndef NDEBUG
        auto const _ = std::scoped_lock(_mutex);
        hi_assert(_thread_ids.empty());
#endif
    }

    template<typename Func>
    callback_impl(Func&& func) : callback_base<ResultType, ArgTypes...>(), _func(std::forward<Func>(func))
    {
    }

    /** Call the callback function.
     *
     * @note A callback is not re-enterable from the same thread.
     * @note It is undefined behavior to destroy a callback while it is in-flight.
     * @param args The arguments forwarded to callback-function.
     * @return The result of the callback-function.
     * @throws The exception thrown from the callback-function.
     */
    ResultType operator()(ArgTypes... args) override
    {
#ifndef NDEBUG
        auto const _ = std::scoped_lock(_mutex);
        auto const thread_id = current_thread_id();
        hi_assert(not std::ranges::contains(_thread_ids, thread_id));
        _thread_ids.push_back(thread_id);
        auto const d = defer([&] {
            std::erase(_thread_ids, thread_id);
        });
#endif

        return _func(args...);
    }

private:
    FunctionType _func;

#ifndef NDEBUG
    mutable std::vector<thread_id> _thread_ids;
    mutable std::mutex _mutex;
#endif
};

} // namespace detail

template<typename T = void()>
class weak_callback;

template<typename T = void()>
class callback;

template<typename ResultType, typename... ArgTypes>
class callback<ResultType(ArgTypes...)>;

template<typename ResultType, typename... ArgTypes>
class weak_callback<ResultType(ArgTypes...)> {
public:
    using result_type = ResultType;
    using callback_type = callback<ResultType(ArgTypes...)>;

    constexpr weak_callback() noexcept = default;
    weak_callback(weak_callback const& other) noexcept = default;
    weak_callback(weak_callback&& other) noexcept = default;
    weak_callback& operator=(weak_callback const& other) noexcept = default;
    weak_callback& operator=(weak_callback&& other) noexcept = default;

    weak_callback(callback_type const& other) noexcept;

    void reset() noexcept
    {
        return _impl.reset();
    }

    [[nodiscard]] long use_count() const noexcept
    {
        return _impl.use_count();
    }

    /** Check if the callback object is expired.
     *
     * @retval false The callback object is functioning.
     * @retval true The callback object is destroyed or in the process of being
     *         destroyed.
     */
    [[nodiscard]] bool expired() const noexcept
    {
        return _impl.expired();
    }

    [[nodiscard]] callback_type lock() const noexcept;

private:
    using base_impl_type = detail::callback_base<ResultType, ArgTypes...>;

    std::weak_ptr<base_impl_type> _impl;
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
template<typename ResultType, typename... ArgTypes>
class callback<ResultType(ArgTypes...)> {
public:
    using result_type = ResultType;
    using weak_callback_type = weak_callback<ResultType(ArgTypes...)>;

    constexpr callback() = default;
    callback(callback const&) = default;
    callback& operator=(callback const&) = default;
    callback(callback&&) = default;
    callback& operator=(callback&&) = default;

    callback(std::nullptr_t) noexcept : _impl(nullptr) {}

    callback& operator=(std::nullptr_t) noexcept
    {
        _impl = nullptr;
        return *this;
    }

    template<typename Func>
    explicit callback(Func&& func) : _impl(std::make_shared<impl_type<Func>>(std::forward<Func>(func)))
    {
    }

    void reset() noexcept
    {
        return _impl.reset();
    }

    [[nodiscard]] long use_count() const noexcept
    {
        return _impl.use_count();
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
    template<typename... Args>
    decltype(auto) operator()(Args... args)
    {
        if (not _impl) {
            throw std::bad_function_call();
        }

        // The callback function does not need to be locked as the callback
        // object can not be destroyed at this point.
        return (*_impl)(std::forward<Args>(args)...);
    }

private:
    using base_impl_type = detail::callback_base<ResultType, ArgTypes...>;

    template<typename FunctionType>
    using impl_type = detail::callback_impl<FunctionType, ResultType, ArgTypes...>;

    std::shared_ptr<base_impl_type> _impl = {};

    callback(std::shared_ptr<base_impl_type> other) noexcept : _impl(std::move(other)) {}

    friend weak_callback<ResultType(ArgTypes...)>;
};

template<typename ResultType, typename... ArgTypes>
hi_inline weak_callback<ResultType(ArgTypes...)>::weak_callback(callback<ResultType(ArgTypes...)> const& other) noexcept :
    _impl(other._impl)
{
}

template<typename ResultType, typename... ArgTypes>
[[nodiscard]] hi_inline callback<ResultType(ArgTypes...)> weak_callback<ResultType(ArgTypes...)>::lock() const noexcept
{
    return {_impl.lock()};
}

} // namespace v1
} // namespace hi::v1
