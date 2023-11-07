// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <coroutine>
#include <type_traits>
#include <memory>
#include <exception>
#include <optional>

export module hikogui_dispatch : scoped_task;
import : notifier;
import hikogui_concurrency_thread; // XXX #616
import hikogui_concurrency_unfair_mutex; // XXX #616
import hikogui_utility;

export namespace hi::inline v1 {

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
    using notifier_type = notifier<void(value_type)>;
    using callback_type = notifier_type::callback_type;

    struct promise_type {
        notifier_type notifier;
        std::optional<value_type> value = {};
        std::exception_ptr exception = nullptr;

        void return_value(std::convertible_to<value_type> auto&& new_value) noexcept
        {
            value = hi_forward(new_value);
        }

        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
        }

        std::suspend_always final_suspend() noexcept
        {
            if (value) {
                // Trigger the notifier with the co_return value.
                notifier(*value);

            } else {
                // Notify also in case of exception.
                hi_assert_not_null(exception);
                if constexpr (std::is_default_constructible_v<value_type>) {
                    notifier(value_type{});
                }
            }

            return {};
        }

        scoped_task get_return_object() noexcept
        {
            return scoped_task{handle_type::from_promise(*this)};
        }

        /** Before we enter the coroutine, allow the caller to set the callback.
         */
        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    scoped_task(handle_type coroutine) noexcept : _coroutine(coroutine) {}

    ~scoped_task()
    {
        if (_coroutine) {
            _coroutine.destroy();
        }
    }

    scoped_task() = default;

    // scoped_task can not be copied because it tracks if the co-routine must be destroyed by the
    // shared_ptr to the value shared between scoped_task and the promise.
    scoped_task(scoped_task const&) = delete;
    scoped_task& operator=(scoped_task const&) = delete;

    scoped_task(scoped_task&& other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
    }

    scoped_task& operator=(scoped_task&& other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        return *this;
    }

    /** Check if the co-routine has completed.
     */
    [[nodiscard]] bool done() const noexcept
    {
        hi_axiom(_coroutine);
        return _coroutine.done();
    }

    /** Check if the co-routine has completed.
     */
    explicit operator bool() const noexcept
    {
        return done();
    }

    /** Get the return value returned from co_return.
     *
     * @note It is undefined behavior to call this function if the co-routine is incomplete.
     * @throws The exception thrown from the co-routine.
     */
    [[nodiscard]] value_type const& value() const
    {
        hi_axiom(done());

        hilet& promise = _coroutine.promise();
        if (promise.value) {
            return *promise.value;

        } else {
            hi_axiom(promise.exception);
            std::rethrow_exception(promise.exception);
        }
    }

    /** Get the return value returned from co_return.
     *
     * @note It is undefined behavior to call this function if the co-routine is incomplete.
     * @throws The exception thrown from the co-routine.
     */
    [[nodiscard]] value_type const& operator*() const
    {
        return value();
    }

    /** Subscribe a callback for when the co-routine is completed.
     *
     * @param flags The callback flags used to be passed to the notifier.
     * @param callback The callback to call when the co-routine executed co_return. If co_return
     *                 has a non-void expression then the callback must accept the expression as an argument.
     * @return The callback token used to manage the lifetime of the callback
     */
    template<forward_of<void(value_type)> Func>
    [[nodiscard]] callback<void(value_type)> subscribe(Func &&func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _coroutine.promise().notifier.subscribe(std::forward<Func>(func), flags);
    }

private:
    // Optional value type
    handle_type _coroutine;
};

/**
 * @sa scoped_task<>
 */
template<>
class scoped_task<void> {
public:
    using value_type = void;
    using notifier_type = notifier<void()>;
    using callback_type = notifier_type::callback_type;

    struct promise_type {
        notifier_type notifier;
        std::exception_ptr exception = nullptr;

        void return_void() noexcept {}

        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
        }

        std::suspend_always final_suspend() noexcept
        {
            notifier();
            return {};
        }

        scoped_task get_return_object() noexcept
        {
            return scoped_task{handle_type::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    scoped_task(handle_type coroutine) noexcept : _coroutine(coroutine) {}

    ~scoped_task()
    {
        if (_coroutine) {
            _coroutine.destroy();
        }
    }

    scoped_task() = default;
    scoped_task(scoped_task const&) = delete;
    scoped_task& operator=(scoped_task const&) = delete;

    scoped_task(scoped_task&& other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
    }

    scoped_task& operator=(scoped_task&& other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        return *this;
    }

    /**
     * @sa scoped_task<>::completed()
     */
    [[nodiscard]] bool done() const noexcept
    {
        hi_axiom(_coroutine);
        return _coroutine.done();
    }

    /**
     * @sa scoped_task<>::operator bool()
     */
    explicit operator bool() const noexcept
    {
        return done();
    }

    /** Get the return value returned from co_return.
     *
     * @note It is undefined behavior to call this function if the co-routine is incomplete.
     * @throws The exception thrown from the co-routine.
     */
    void value() const
    {
        hi_axiom(done());

        hilet& promise = _coroutine.promise();
        if (promise.exception) {
            std::rethrow_exception(promise.exception);
        }
    }

    /**
     * @sa notifier<>::subscribe()
     */
    template<forward_of<void()> Func>
    [[nodiscard]] callback<void()> subscribe(Func &&func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _coroutine.promise().notifier.subscribe(std::forward<Func>(func), flags);
    }

private:
    // Optional value type
    handle_type _coroutine;
};

} // namespace hi::inline v1
