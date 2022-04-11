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

namespace tt::inline v1 {

/** A notifier which can be used to call a set of registered callbacks.
 *
 * @tparam Result The result of calling the callback.
 * @tparam Args The argument types of the callback function.
 */
template<typename T = void()>
class notifier {
};

// The partial template specialization allows the use of a `std::function`-like template
// argument, that looks like a function prototype.
template<typename Result, typename... Args>
class notifier<Result(Args...)> {
public:
    static_assert(std::is_same_v<Result, void>, "Result of a notifier must be void.");

    using result_type = Result;
    using callback_type = std::function<Result(Args const&...)>;

    using token_type = std::shared_ptr<callback_type>;
    using weak_token_type = std::weak_ptr<callback_type>;

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
            tt_axiom(_notifier != nullptr);

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
     * caller will receive a token, a move-only RAII object that will unsubscribe the callback
     * when the token is destroyed.
     *
     * @param callback_ptr A shared_ptr to a callback function.
     * @return A RAII object which when destroyed will unsubscribe the callback.
     */
    [[nodiscard]] token_type subscribe(std::invocable<Args...> auto&& callback) noexcept
    {
        auto token = std::make_shared<callback_type>(tt_forward(callback));
        _callbacks.emplace_back(token);
        return token;
    }

    /** Call the subscribed callbacks synchronously with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    void call(Args const&...args) const noexcept requires(std::is_same_v<result_type, void>)
    {
#if TT_BUILD_TYPE == TT_BT_DEBUG
        tt_axiom(std::exchange(_notifying, true) == false);
#endif
        ttlet tmp = _callbacks;
        for (auto& weak_callback : tmp) {
            if (auto callback = weak_callback.lock()) {
                (*callback)(args...);
            }
        }
#if TT_BUILD_TYPE == TT_BT_DEBUG
        _notifying = false;
#endif
    }

    /** Post the subscribed callbacks on the current thread's event loop with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    void post(Args const&...args) const noexcept requires(std::is_same_v<result_type, void>)
    {
        for (auto& weak_callback : _callbacks) {
            loop::local().post_function([=] {
                if (auto callback = weak_callback.lock()) {
                    (*callback)(args...);
                }
            });
        }
    }

    /** Post the subscribed callbacks on the main thread's event loop with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    void post_on_main(Args const&...args) const noexcept requires(std::is_same_v<result_type, void>)
    {
        for (auto& weak_callback : _callbacks) {
            loop::main().post_function([=] {
                if (auto callback = weak_callback.lock()) {
                    (*callback)(args...);
                }
            });
        }
    }

    /** Call the subscribed callbacks with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    auto operator()(Args const&...args) const noexcept
    {
        return post(args...);
    }

private:
    /** A list of callbacks and it's associated token.
     */
    std::vector<weak_token_type> _callbacks;

#if TT_BUILD_TYPE == TT_BT_DEBUG
    /** The notifier is currently calling all the callbacks.
     */
    mutable bool _notifying = false;
#endif
};

} // namespace tt::inline v1
