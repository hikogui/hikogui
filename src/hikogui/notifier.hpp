// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "generator.hpp"
#include "loop.hpp"
#include <vector>
#include <tuple>
#include <functional>
#include <coroutine>

namespace hi::inline v1 {

/** A notifier which holds a set of callbacks.
 *
 * @tparam Proto The prototype of the callbacks, like: `std::function<Proto>`.
 */
template<typename Proto = void()>
class notifier {
};

/** A notifier which holds a set of callbacks.
 *
 * @tparam Result The result type of the callbacks.
 * @tparam Args... The argument types of the callbacks.
 */
template<typename Result, typename... Args>
class notifier<Result(Args...)> {
public:
    // XXX Change this class to handle non-void return type.
    static_assert(std::is_same_v<Result, void>, "Result of a notifier must be void.");

    /** The result type of the callback.
     */
    using result_type = Result;

    /** The function type that is stored inside the notifier.
     */
    using function_type = std::function<Result(Args const&...)>;

    /** A token that represents a subscribed function.
     *
     * @note When all copies of a token are destroyed the subscription is removed.
     * @note Behaves like a function pointer and can be dereferenced to be called.
     */
    using token_type = std::shared_ptr<function_type>;

    /** A weak-token that represents a subscribed function.
     *
     * @note You can use the `lock()` member function to get an actual token.
     */
    using weak_token_type = std::weak_ptr<function_type>;

    /** An awaiter object which can wait on a notifier.
     *
     * When this object is awaited on a callback to this object is created
     * with the associated notifier. This allows a co-routine that awaits
     * on this object to be resumed.
     */
    class awaiter_type {
    public:
        constexpr awaiter_type() noexcept = default;
        constexpr awaiter_type(awaiter_type const&) noexcept = default;
        constexpr awaiter_type(awaiter_type&&) noexcept = default;
        constexpr awaiter_type& operator=(awaiter_type const&) noexcept = default;
        constexpr awaiter_type& operator=(awaiter_type&&) noexcept = default;

        constexpr awaiter_type(notifier& notifier) noexcept : _notifier(&notifier) {}

        [[nodiscard]] constexpr bool await_ready() noexcept
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) noexcept
        {
            hi_axiom(_notifier != nullptr);

            // We can use the this pointer in the callback, as `await_suspend()` is called by
            // the co-routine on the same object as `await_resume()`.
            _cbt = _notifier->subscribe([this, handle](Args const&...args) {
                _args = {args...};
                handle.resume();
            });
        }

        constexpr void await_resume() const noexcept requires(sizeof...(Args) == 0) {}

        constexpr auto await_resume() const noexcept requires(sizeof...(Args) == 1)
        {
            return std::get<0>(_args);
        }

        constexpr auto await_resume() const noexcept requires(sizeof...(Args) > 1)
        {
            return _args;
        }

        [[nodiscard]] bool operator==(awaiter_type const& rhs) const noexcept
        {
            return _notifier == rhs._notifier;
        }

    private:
        notifier *_notifier = nullptr;
        token_type _cbt;
        std::tuple<Args...> _args;
    };

    /** Create a notifier.
     */
    constexpr notifier() noexcept = default;
    notifier(notifier&&) = delete;
    notifier(notifier const&) = delete;
    notifier& operator=(notifier&&) = delete;
    notifier& operator=(notifier const&) = delete;

    /** Create an awaiter that can await on this notifier.
     */
    awaiter_type operator co_await() const noexcept
    {
        return awaiter_type{const_cast<notifier&>(*this)};
    }

    /** Add a callback to the notifier.
     * Ownership of the callback belongs with the caller of `subscribe()`. The
     * caller will receive a token, a RAII object that will unsubscribe the callback
     * when the token is destroyed.
     *
     * @param callback A callback function.
     * @return A RAII object which when destroyed will unsubscribe the callback.
     */
    [[nodiscard]] token_type subscribe(std::invocable<Args...> auto&& callback) noexcept
    {
        auto token = std::make_shared<function_type>(hi_forward(callback));
        hilet lock = std::scoped_lock(_mutex);
        _callbacks.emplace_back(token);
        return token;
    }

    /** Post the subscribed callbacks on the local-thread's event loop with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    void post(Args const&...args) const noexcept requires(std::is_same_v<result_type, void>)
    {
        handle_callbacks([&](token_type token) {
            loop::local().post_function([=] {
                (*token)(args...);
            });
        });
    }

    /** Post the subscribed callbacks on the main-thread's event loop with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    void post_on_main(Args const&...args) const noexcept requires(std::is_same_v<result_type, void>)
    {
        handle_callbacks([&](token_type token) {
            loop::main().post_function([=] {
                (*token)(args...);
            });
        });
    }

private:
    /** Mutex to manage the callbacks.
     */
    mutable unfair_mutex _mutex;

    /** A list of callbacks and it's associated token.
     */
    mutable std::vector<weak_token_type> _callbacks;

    void handle_callbacks(auto&& func) const noexcept
    {
        hilet lock = std::scoped_lock(_mutex);

        auto it = _callbacks.begin();
        auto last = _callbacks.end();

        while (it != last) {
            if (auto token = it->lock()) {
                func(std::move(token));
                ++it;

            } else {
                std::swap(*it, *(last - 1));
                --last;
            }
        }

        _callbacks.erase(last, _callbacks.end());
    }
};

} // namespace hi::inline v1
