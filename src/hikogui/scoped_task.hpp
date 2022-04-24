// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "notifier.hpp"
#include "counters.hpp"
#include <coroutine>
#include <type_traits>
#include <memory>
#include <exception>

namespace hi::inline v1 {

/** A scoped_task.
 *
 * Like the `hi::task` instance this implements a asynchronous co-routine task.
 *
 * If the `scoped_task` object is destroyed, the potentially non-completed co-routine will be destroyed as well.
 * A `scoped_task` is a move-only object.
 *
 * @tparam T The type returned by co_return.
 */
template<typename T = void>
class scoped_task {
public:
    using value_type = T;

    /** The return value type.
     * This value is shared between the promise and the scoped_task.
     * The variant has three different states:
     *  - 0: co-routine has not completed.
     *  - 1: co-routine caught an uncaught exception.
     *  - 2: co-routine is completed with a value.
     */
    using return_value_type = std::variant<std::monostate, std::exception_ptr, value_type>;
    using return_value_ptr_type = std::shared_ptr<return_value_type>;
    using const_return_value_ptr_type = std::shared_ptr<return_value_type const>;
    using notifier_type = notifier<void(value_type)>;

    struct promise_type {
        notifier_type _notifier;
        return_value_ptr_type _value_ptr;

        void return_value(std::convertible_to<value_type> auto &&value) noexcept
        {
            *_value_ptr = return_value_type{std::in_place_index<2>, hi_forward(value)};
        }

        void unhandled_exception() noexcept
        {
            *_value_ptr = return_value_type{std::in_place_index<1>, std::current_exception()};
        }

        std::suspend_never final_suspend() noexcept
        {
            switch (_value_ptr->index()) {
            case 1:
                // We need to trigger the notifier on an exception too.
                if constexpr (std::is_default_constructible_v<value_type>) {
                    _notifier(value_type{});
                }
                return {};
            case 2:
                // Trigger the notifier with the co_return value.
                _notifier(std::get<2>(*_value_ptr));
                return {};
            default: hi_no_default();
            }
        }

        scoped_task get_return_object() noexcept
        {
            _value_ptr = std::make_shared<return_value_type>();
            return scoped_task{handle_type::from_promise(*this), _value_ptr};
        }

        /** Before we enter the coroutine, allow the caller to set the callback.
         */
        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    scoped_task(handle_type coroutine, const_return_value_ptr_type value_ptr) noexcept :
        _coroutine(coroutine), _value_ptr(std::move(value_ptr))
    {
    }

    ~scoped_task()
    {
        if (_value_ptr and not completed()) {
            hi_axiom(_coroutine);
            _coroutine.destroy();
        }
    }

    scoped_task() = default;

    // scoped_task can not be copied because it tracks if the co-routine must be destroyed by the
    // shared_ptr to the value shared between scoped_task and the promise.
    scoped_task(scoped_task const &) = delete;
    scoped_task &operator=(scoped_task const &) = delete;

    scoped_task(scoped_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _value_ptr = std::exchange(other._value_ptr, {});
    }

    scoped_task &operator=(scoped_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _value_ptr = std::exchange(other._value_ptr, {});
        return *this;
    }

    /** Check if the co-routine has completed.
     */
    [[nodiscard]] bool completed() const noexcept
    {
        return _value_ptr->index() != 0;
    }

    /** Check if the co-routine has completed.
     */
    explicit operator bool() const noexcept
    {
        return completed();
    }

    /** Get the return value returned from co_return.
     *
     * @note It is undefined behavior to call this function if the co-routine is incomplete.
     * @throws The exception thrown from the co-routine.
     */
    [[nodiscard]] value_type const &value() const
    {
        switch (_value_ptr->index()) {
        case 1: std::rethrow_exception(std::get<1>(*_value_ptr));
        case 2: return std::get<2>(*_value_ptr);
        default: hi_no_default();
        }
    }

    /** Get the return value returned from co_return.
     *
     * @note It is undefined behavior to call this function if the co-routine is incomplete.
     * @throws The exception thrown from the co-routine.
     */
    [[nodiscard]] value_type const &operator*() const
    {
        return value();
    }

    /** Subscribe a callback for when the co-routine is completed.
     *
     * @param callback The callback to call when the co-routine executed co_return. If co_return
     *                 has a non-void expression then the callback must accept the expression as an argument.
     */
    notifier_type::token_type subscribe(std::invocable<value_type> auto &&callback) noexcept
    {
        return _coroutine.promise()._notifier.subscribe(hi_forward(callback));
    }

private:
    // Optional value type
    handle_type _coroutine;
    const_return_value_ptr_type _value_ptr;
};

/**
 * @sa scoped_task<>
 */
template<>
class scoped_task<void> {
public:
    using value_type = void;

    /** The return value type.
     * This value is shared between the promise and the scoped_task.
     * The variant has three different states:
     *  - 0: co-routine has not completed.
     *  - 1: co-routine caught an uncaught exception.
     *  - 2: co-routine is completed.
     */
    using return_value_type = std::variant<std::monostate, std::exception_ptr, std::monostate>;
    using return_value_ptr_type = std::shared_ptr<return_value_type>;
    using const_return_value_ptr_type = std::shared_ptr<return_value_type const>;
    using notifier_type = notifier<void()>;

    struct promise_type {
        notifier_type _notifier;
        return_value_ptr_type _value_ptr;

        void return_void() noexcept
        {
            *_value_ptr = return_value_type{std::in_place_index<2>};
        }

        void unhandled_exception() noexcept
        {
            *_value_ptr = return_value_type{std::in_place_index<1>, std::current_exception()};
        }

        std::suspend_never final_suspend() noexcept
        {
            switch (_value_ptr->index()) {
            case 1:
                // Trigger the notifier on exception.
                _notifier();
                return {};
            case 2:
                // Trigger the notifier with the co_return value.
                _notifier();
                return {};
            default: hi_no_default();
            }
        }
        scoped_task get_return_object() noexcept
        {
            _value_ptr = std::make_shared<return_value_type>();
            return scoped_task{handle_type::from_promise(*this), _value_ptr};
        }

        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    scoped_task(handle_type coroutine, const_return_value_ptr_type value_ptr) noexcept :
        _coroutine(coroutine), _value_ptr(std::move(value_ptr))
    {
    }

    ~scoped_task()
    {
        if (_value_ptr and not completed()) {
            hi_axiom(_coroutine);
            _coroutine.destroy();
        }
    }

    scoped_task() = default;
    scoped_task(scoped_task const &) = delete;
    scoped_task &operator=(scoped_task const &) = delete;

    scoped_task(scoped_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _value_ptr = std::exchange(other._value_ptr, {});
    }

    scoped_task &operator=(scoped_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _value_ptr = std::exchange(other._value_ptr, {});
        return *this;
    }

    /**
     * @sa scoped_task<>::completed()
     */
    [[nodiscard]] bool completed() const noexcept
    {
        return _value_ptr->index() != 0;
    }

    /**
     * @sa scoped_task<>::operator bool()
     */
    explicit operator bool() const noexcept
    {
        return completed();
    }

    /** Get the return value returned from co_return.
     *
     * @note It is undefined behavior to call this function if the co-routine is incomplete.
     * @return void
     * @throws The exception thrown from the co-routine.
     */
    void value() const
    {
        switch (_value_ptr->index()) {
        case 1: std::rethrow_exception(std::get<1>(*_value_ptr));
        case 2: return;
        default: hi_no_default();
        }
    }

    /**
     * @sa scoped_task<>::subscribe()
     */
    notifier_type::token_type subscribe(std::invocable<> auto &&callback) noexcept
    {
        return _coroutine.promise()._notifier.subscribe(hi_forward(callback));
    }

private:
    // Optional value type
    handle_type _coroutine;
    const_return_value_ptr_type _value_ptr;
};

} // namespace hi::inline v1
