// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "generator.hpp"
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

    /** Object that represents a `notifier` callback token.
     *
     * When this object is destroyed it will unsubscribe the associated callback.
     */
    class token_type {
    public:
        ~token_type()
        {
            if (_notifier) {
                _notifier->remove_token(*this);
            }
        }

        /** Default constructor.
         * This constructor creates a empty token which is not assocciated with a callback.
         */
        constexpr token_type() noexcept = default;

        constexpr token_type(token_type const& other) noexcept : _notifier(nullptr)
        {
            if (other._notifier) {
                other._notifier->copy_token(other, *this);
                _notifier = other._notifier;
            }
        }

        constexpr token_type& operator=(token_type const& other) noexcept
        {
            tt_return_on_self_assignment(other);

            if (_notifier) {
                _notifier->remove_token(*this);
                _notifier = nullptr;
            }
            if (other._notifier) {
                other._notifier->copy_token(other, *this);
                _notifier = other._notifier;
            }
            return *this;
        }

        constexpr token_type(token_type&& other) noexcept : _notifier(nullptr)
        {
            if (other._notifier) {
                other._notifier->move_token(other, *this);
                _notifier = std::exchange(other._notifier, nullptr);
            }
        }

        constexpr token_type& operator=(token_type&& other) noexcept
        {
            tt_return_on_self_assignment(other);

            if (_notifier) {
                _notifier->remove_token(*this);
                _notifier = nullptr;
            }
            if (other._notifier) {
                other._notifier->move_token(other, *this);
                _notifier = std::exchange(other._notifier, nullptr);
            }
            return *this;
        }

        constexpr token_type(notifier *notifier) noexcept : _notifier(notifier) {}

        constexpr auto operator()(Args const&...args) noexcept
        {
            tt_axiom(_notifier);
            return (*_notifier)(args...);
        }

    private:
        notifier *_notifier = nullptr;
    };

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
            _cbt = _notifier->subscribe([handle] {
                handle.resume();
            });
        }

        constexpr result_type await_resume() const noexcept {}

        [[nodiscard]] bool operator==(awaiter_type const& rhs) const noexcept
        {
            return _notifier == rhs._notifier;
        }

    private:
        notifier *_notifier = nullptr;
        token_type _cbt;
    };

    /** Create a notifier.
     */
    constexpr notifier() noexcept = default;
    notifier(notifier&&) = delete;
    notifier(notifier const&) = delete;
    notifier& operator=(notifier&&) = delete;
    notifier& operator=(notifier const&) = delete;

    /** Destroys a notifier.
     * This will empty any token that was associated with this notifier.
     */
    ~notifier()
    {
        for (auto& callback : _callbacks) {
            const_cast<token_type *>(callback.first)->_notifier = nullptr;
        }
    }

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
        auto sub = token_type{this};
        _callbacks.emplace_back(&sub, tt_forward(callback));
        return sub;
    }

    /** Call the subscribed callbacks with the given arguments.
     * 
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     */
    void operator()(Args const&...args) const noexcept requires(std::is_same_v<result_type, void>)
    {
#if TT_BUILD_TYPE == TT_BT_DEBUG
        tt_axiom(std::exchange(_notifying, true) == false);
#endif
        ttlet tmp = _callbacks;
        for (auto& callback : tmp) {
            callback.second(args...);
        }
#if TT_BUILD_TYPE == TT_BT_DEBUG
        _notifying = false;
#endif
    }

    /** Call the subscribed callbacks with the given arguments.
     *
     * @note This function is not reentrant.
     * @param args The arguments to pass with the invocation of the callback
     * @return The result of each callback.
     */
    generator<result_type> operator()(Args const&...args) const noexcept requires(not std::is_same_v<result_type, void>)
    {
#if TT_BUILD_TYPE == TT_BT_DEBUG
        tt_axiom(std::exchange(_notifying, true) == false);
#endif
        ttlet tmp = _callbacks;
        for (auto& callback : tmp) {
            co_yield callback.second(args...);
        }
#if TT_BUILD_TYPE == TT_BT_DEBUG
        _notifying = false;
#endif
    }

private:
    /** A list of callbacks and it's associated token.
     */
    std::vector<std::pair<token_type const *, callback_type>> _callbacks;

#if TT_BUILD_TYPE == TT_BT_DEBUG
    /** The notifier is currently calling all the callbacks.
     */
    mutable bool _notifying = false;
#endif

    /** Remove the token from the callback list.
     */
    void remove_token(token_type const& sub) noexcept
    {
        ttlet erase_count = std::erase_if(_callbacks, [&](ttlet& item) {
            return item.first == &sub;
        });
        tt_axiom(erase_count == 1);
    }

    /** Handle move-assign and move-construct of a token.
     */
    void move_token(token_type const& sub, token_type const& new_sub) noexcept
    {
        ttlet it = std::find_if(_callbacks.begin(), _callbacks.end(), [&](ttlet& item) {
            return item.first == &sub;
        });
        tt_axiom(it != _callbacks.end());
        it->first = &new_sub;
    }

    /** Handle copy-assign and copy-construt of a token.
     */
    void copy_token(token_type const& sub, token_type const& new_sub) noexcept
    {
        ttlet it = std::find_if(_callbacks.begin(), _callbacks.end(), [&](ttlet& item) {
            return item.first == &sub;
        });
        tt_axiom(it != _callbacks.end());
        _callbacks.emplace_back(&new_sub, it->second);
    }
};

} // namespace tt::inline v1

