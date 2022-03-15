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

namespace tt::inline v1 {

/** A delayed_task.
 *
 * Like the `tt::task` instance this implements a asynchronous co-routine task.
 * This delayed variant will immediately return to the caller until `delayed_task::resume()` is
 * called on this. This will allow this object to be assigned callbacks to trigger when co_return
 * is called.
 *
 * The `delayed_task` object needs to be held by the caller until the co-routine returns.
 * If the `delayed_task` object is destroyed, the co-routine will be destroyed as well.
 *
 * @tparam T The type returned by co_return.
 */
template<typename T = void>
class delayed_task {
public:
    using value_type = T;

    using optional_value_type = std::optional<value_type>;
    using optional_value_ptr_type = std::shared_ptr<optional_value_type>;
    using notifier_type = notifier<void(value_type)>;

    struct promise_type {
        notifier_type _notifier;
        optional_value_ptr_type _optional_value_ptr;

        void return_value(std::convertible_to<value_type> auto &&value) noexcept
        {
            *_optional_value_ptr = tt_forward(value);
        }

        std::suspend_never final_suspend() noexcept
        {
            // After final suspend this promise will be destroyed.
            // But the notifier may try to manually destroy the co-routine (and this promise)
            // if the promise is still holding on to the _optional_value_ptr.
            auto value = **_optional_value_ptr;
            _optional_value_ptr.reset();
            _notifier(std::move(value));
            return {};
        }

        void unhandled_exception()
        {
            throw;
        }

        delayed_task get_return_object() noexcept
        {
            _optional_value_ptr = std::make_shared<optional_value_type>();
            return delayed_task{handle_type::from_promise(*this), _optional_value_ptr};
        }

        /** Before we enter the coroutine, allow the caller to set the callback.
         */
        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    delayed_task(handle_type coroutine, std::shared_ptr<optional_value_type> value_ptr) noexcept :
        _coroutine(coroutine), _optional_value_ptr(std::move(value_ptr))
    {
    }

    ~delayed_task()
    {
        if (_optional_value_ptr) {
            if (_optional_value_ptr.use_count() > 1) {
                tt_axiom(_coroutine);
                _coroutine.destroy();
            }
            tt_axiom(_optional_value_ptr.use_count() == 1);
        }
    }

    delayed_task() = default;
    delayed_task(delayed_task const &) = delete;
    delayed_task &operator=(delayed_task const &) = delete;

    delayed_task(delayed_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _optional_value_ptr = std::exchange(other._optional_value_ptr, {});
    }

    delayed_task &operator=(delayed_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _optional_value_ptr = std::exchange(other._optional_value_ptr, {});
        return *this;
    }

    [[nodiscard]] bool completed() const noexcept
    {
        return static_cast<bool>(*_optional_value_ptr);
    }

    explicit operator bool() const noexcept
    {
        return completed();
    }

    [[nodiscard]] value_type const &value() const noexcept
    {
        return **_optional_value_ptr;
    }

    [[nodiscard]] value_type const &operator*() const noexcept
    {
        return value();
    }

    /** Resume the co-routine.
     *
     * The co-routine initially suspends, which allows a callback to be
     * attached to the co-routine which will be called the co-routine returns.
     *
     * @param callback The callback to be called when the co-routine returns.
     * @return A callback-token used to track the lifetime of the callback.
     */
    auto resume(std::invocable<value_type> auto &&callback) noexcept
    {
        auto tmp = _coroutine.promise()._notifier.subscribe(tt_forward(callback));
        _coroutine.resume();
        return tmp;
    }

private:
    // Optional value type
    handle_type _coroutine;
    optional_value_ptr_type _optional_value_ptr;
};

template<>
class delayed_task<void> {
public:
    using value_type = void;
    using optional_value_type = bool;
    using optional_value_ptr_type = std::shared_ptr<optional_value_type>;
    using notifier_type = notifier<void()>;

    struct promise_type {
        notifier_type _notifier;
        optional_value_ptr_type _optional_value_ptr;

        void return_void() noexcept
        {
            *_optional_value_ptr = true;
        }

        std::suspend_never final_suspend() noexcept
        {
            // After final suspend this promise will be destroyed.
            // But the notifier may try to manually destroy the co-routine (and this promise)
            // if the promise is still holding on to the _optional_value_ptr.
            _optional_value_ptr.reset();
            _notifier();
            return {};
        }

        void unhandled_exception()
        {
            throw;
        }

        delayed_task get_return_object() noexcept
        {
            _optional_value_ptr = std::make_shared<optional_value_type>();
            return delayed_task{handle_type::from_promise(*this), _optional_value_ptr};
        }

        /** Before we enter the coroutine, allow the caller to set the callback.
         */
        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    delayed_task(handle_type coroutine, std::shared_ptr<optional_value_type> value_ptr) noexcept :
        _coroutine(coroutine), _optional_value_ptr(std::move(value_ptr))
    {
    }

    ~delayed_task()
    {
        if (_optional_value_ptr) {
            if (_optional_value_ptr.use_count() > 1) {
                tt_axiom(_coroutine);
                _coroutine.destroy();
            }
            tt_axiom(_optional_value_ptr.use_count() == 1);
        }
    }

    delayed_task() = default;
    delayed_task(delayed_task const &) = delete;
    delayed_task &operator=(delayed_task const &) = delete;

    delayed_task(delayed_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _optional_value_ptr = std::exchange(other._optional_value_ptr, {});
    }

    delayed_task &operator=(delayed_task &&other) noexcept
    {
        _coroutine = std::exchange(other._coroutine, {});
        _optional_value_ptr = std::exchange(other._optional_value_ptr, {});
        return *this;
    }

    [[nodiscard]] bool completed() const noexcept
    {
        return static_cast<bool>(*_optional_value_ptr);
    }

    explicit operator bool() const noexcept
    {
        return completed();
    }

    /** Resume the co-routine.
     *
     * The co-routine initially suspends, which allows a callback to be
     * attached to the co-routine which will be called the co-routine returns.
     *
     * @param callback The callback to be called when the co-routine returns.
     * @return A callback-token used to track the lifetime of the callback.
     */
    auto resume(std::invocable<> auto &&callback) noexcept
    {
        auto tmp = _coroutine.promise()._notifier.subscribe(tt_forward(callback));
        _coroutine.resume();
        return tmp;
    }

private:
    // Optional value type
    handle_type _coroutine;
    optional_value_ptr_type _optional_value_ptr;
};

} // namespace tt::inline v1
