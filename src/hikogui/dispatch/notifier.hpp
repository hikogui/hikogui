// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../concurrency/concurrency.hpp"
#include "../macros.hpp"
#include <vector>
#include <tuple>
#include <functional>
#include <coroutine>
#include <mutex>

hi_export_module(hikogui.dispatch : notifier);

hi_export namespace hi::inline v1 {

/** A notifier which can be used to call a set of registered callbacks.
 *
 * @tparam Result The result of calling the callback.
 * @tparam Args The argument types of the callback function.
 */
template<typename T = void()>
class notifier;

// The partial template specialization allows the use of a `std::function`-like template
// argument, that looks like a function prototype.
template<typename R, typename... Args>
class notifier<R(Args...)> {
public:
    static_assert(std::is_same_v<R, void>, "Result of a notifier must be void.");

    using result_type = R;
    using callback_proto = R(Args...);

    using callback_type = callback<R(Args...)>;
    using weak_callback_type = weak_callback<R(Args...)>;

    /** An awaiter object which can wait on a notifier.
     *
     * When this object is awaited on a callback to this object is created
     * with the associated notifier. This allows a co-routine that awaits
     * on this object to be resumed.
     */
    class awaiter_type {
    public:
        constexpr awaiter_type() noexcept = default;
        constexpr awaiter_type(awaiter_type&&) noexcept = default;
        constexpr awaiter_type& operator=(awaiter_type const&) noexcept = delete;

        /** Copy the awaitable.
         *
         * This only copies the pointer to the notifier. The arguments to the
         * notifier and the callback object are unset.
         */
        constexpr awaiter_type(awaiter_type const& other) noexcept : _notifier(other._notifier) {}

        /** Copy the awaitable.
         *
         * This only copies the pointer to the notifier. The arguments to the
         * notifier and the callback object are reset.
         */
        constexpr awaiter_type& operator=(awaiter_type&&other) noexcept
        {
            _cbt = nullptr;
            _args = {};
            _notifier = other._notifier;
        }

        constexpr awaiter_type(notifier& notifier) noexcept : _notifier(&notifier) {}

        [[nodiscard]] constexpr bool await_ready() noexcept
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) noexcept
        {
            hi_assert_not_null(_notifier);

            // We can use the this pointer in the callback, as `await_suspend()` is called by
            // the co-routine on the same object as `await_resume()`.
            _cbt = _notifier->subscribe(
                [this, handle](Args... args) {
                    // Copy the arguments received from the notifier into the awaitable object
                    // So that it can be read using `await_resume()`.
                    _args = {std::forward<Args>(args)...};

                    // Take ownership of the awaiter's callback and
                    // unsubscribe, so that the awaiter can be destroyed when
                    // its co-routine is resumed.
                    //
                    // We can unsubscribe unsafely here as this is the only
                    // one call inflight due to callback_flags::once.
                    auto my_frame = std::exchange(this->_cbt, nullptr).unsafe_unsubscribe();

                    // Resume the co-routine.
                    handle.resume();
                },
                callback_flags::main | callback_flags::once);
        }

        constexpr void await_resume() const noexcept
            requires(sizeof...(Args) == 0)
        {
        }

        constexpr auto await_resume() const noexcept
            requires(sizeof...(Args) == 1)
        {
            return std::get<0>(_args);
        }

        constexpr auto await_resume() const noexcept
            requires(sizeof...(Args) > 1)
        {
            return _args;
        }

    private:
        notifier *_notifier = nullptr;
        callback<R(Args...)> _cbt;
        std::tuple<Args...> _args;
    };

    /** Create a notifier.
     */
    constexpr notifier() noexcept = default;
    constexpr notifier(notifier&&) noexcept = default;
    constexpr notifier(notifier const&) noexcept = default;
    constexpr notifier& operator=(notifier&&) noexcept = default;
    constexpr notifier& operator=(notifier const&) noexcept = default;

    /** Create an awaiter that can await on this notifier.
     */
    awaiter_type operator co_await() const noexcept
    {
        return awaiter_type{const_cast<notifier&>(*this)};
    }

    /** Add a callback to the notifier.
     *
     * After the call the caller will take ownership of the returned callback
     * object.
     *
     * The `callback` object is a move-only RAII object that will automatically
     * unsubscribe the callback when the token is destroyed.
     *
     * @param flags The callback-flags used to determine how the @a callback is called.
     * @param callback A function object to call when being notified.
     * @return A RAII object which when destroyed will unsubscribe the callback.
     */
    template<forward_of<callback_proto> Func>
    [[nodiscard]] callback_type subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        auto callback = callback_type{std::forward<Func>(func)};
        hilet lock = std::scoped_lock(_mutex);
        _callbacks.emplace_back(callback, flags);
        return callback;
    }

    template<forward_of<void()> F>
    void loop_local_post_function(F&&) const noexcept;
    template<forward_of<void()> F>
    void loop_main_post_function(F&&) const noexcept;
    template<forward_of<void()> F>
    void loop_timer_post_function(F&&) const noexcept;

    /** Call the subscribed callbacks with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    void operator()(Args... args) const noexcept
    {
        hilet lock = std::scoped_lock(_mutex);

        for (auto& [callback, flags] : _callbacks) {
            if (is_synchronous(flags)) {
                if (callback.lock()) {
                    callback(std::forward<Args>(args)...);
                    callback.unlock();
                }

            } else if (is_local(flags)) {
                loop_local_post_function([=] {
                    // The callback object here is captured by-copy, so that
                    // the loop can check if it was expired.
                    if (callback.lock()) {
                        // The captured arguments are now plain copies so we do
                        // not forward them in the call.
                        callback(args...);
                        callback.unlock();
                    }
                });

            } else if (is_main(flags)) {
                loop_main_post_function([=] {
                    // The callback object here is captured by-copy, so that
                    // the loop can check if it was expired.
                    if (callback.lock()) {
                        // The captured arguments are now plain copies so we do
                        // not forward them in the call.
                        callback(args...);
                        callback.unlock();
                    }
                });

            } else if (is_timer(flags)) {
                loop_timer_post_function([=] {
                    // The callback object here is captured by-copy, so that
                    // the loop can check if it was expired.
                    if (callback.lock()) {
                        // The captured arguments are now plain copies so we do
                        // not forward them in the call.
                        callback(args...);
                        callback.unlock();
                    }
                });

            } else {
                hi_no_default();
            }

            // If the callback should only be triggered once, like inside an awaitable.
            // Then reset the weak_ptr in _callbacks so that it will be cleaned up.
            // In the lambda above the weak_ptr is copied first so that it callback will get executed
            // as long as the shared_ptr's use count does not go to zero.
            if (is_once(flags)) {
                callback.reset();
            }
        }
        clean_up();
    }

private:
    mutable unfair_mutex _mutex;

    /** A list of callbacks and it's associated token.
     */
    mutable std::vector<std::pair<weak_callback_type, callback_flags>> _callbacks;

    void clean_up() const noexcept
    {
        hi_axiom(_mutex.is_locked());

        // Cleanup all callbacks that have expired, or when they may only be triggered once.
        std::erase_if(_callbacks, [](hilet& item) {
            return item.first.expired();
        });
    }

#ifndef NDEBUG
    /** The notifier is currently calling all the callbacks.
     */
    mutable bool _notifying = false;
#endif
};

} // namespace hi::inline v1
