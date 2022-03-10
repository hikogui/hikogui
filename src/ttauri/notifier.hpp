// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "unfair_recursive_mutex.hpp"
#include "coroutine.hpp"
#include "awaiter.hpp"
#include <mutex>
#include <vector>
#include <tuple>
#include <functional>
#include <coroutine>

namespace tt::inline v1 {

template<typename T>
class notifier {
};

template<typename Notifier>
class notifier_awaiter;

/** A notifier which can be used to call a set of registered callbacks.
 * This class is thread-safe; however you must not use this object
 * from within the callback.
 *
 * @tparam Result The result of calling the callback.
 * @tparam Args The argument types of the callback function.
 */
template<typename Result, typename... Args>
class notifier<Result(Args...)> {
public:
    static_assert(std::is_same_v<Result, void>, "Result of a notifier must be void.");

    using result_type = Result;
    using awaiter_type = notifier_awaiter<notifier>;
    using callback_type = std::function<Result(Args const &...)>;

    /** Object that represents a callback subscription.
     */
    class subscription {
    public:
        ~subscription()
        {
            if (_notifier) {
                _notifier->unsubscribe(this);
            }
        }

        constexpr subscription() noexcept : _notifier(nullptr) {}

        constexpr subscription(subscription const &other) noexcept : _notifier(nullptr)
        {
            if (other._notifier) {
                other._notifier->copy_subscription(&other, this);
                _notifier = other._notifier;
            }
        }

        constexpr subscription &operator=(subscription const &other) noexcept
        {
            tt_return_on_self_assignment(other);

            if (_notifier) {
                _notifier->unsubscribe(this);
                _notifier = nullptr;
            }
            if (other._notifier) {
                other._notifier->copy_subscription(&other, this);
                _notifier = other._notifier;
            }
            return *this;
        }

        constexpr subscription(subscription &&other) noexcept : _notifier(nullptr)
        {
            if (other._notifier) {
                other._notifier->move_subscription(&other, this);
                _notifier = std::exchange(other._notifier, nullptr);
            }
        }

        constexpr subscription &operator=(subscription &&other) noexcept
        {
            tt_return_on_self_assignment(other);

            if (_notifier) {
                _notifier->unsubscribe(this);
                _notifier = nullptr;
            }
            if (other._notifier) {
                other._notifier->move_subscription(&other, this);
                _notifier = std::exchange(other._notifier, nullptr);
            }
            return *this;
        }

        constexpr subscription(notifier *notifier) noexcept : _notifier(notifier){}

    private:
        notifier *_notifier;

        friend notifier;
    };

    constexpr notifier() noexcept = default;

    ~notifier()
    {
        for (auto &callback: _callbacks) {
            const_cast<subscription *>(callback.first)->_notifier = nullptr;
        }
    }

    awaiter_type operator co_await() const noexcept
    {
        return awaiter_type{const_cast<notifier &>(*this)};
    }

    /** Add a callback to the notifier.
     * Ownership of the callback belongs with the caller of `subscribe()`. The
     * `notifier` will hold a weak_ptr to the callback so that when the callback is destroyed
     * it will no longer be called.
     *
     * @param callback_ptr A shared_ptr to a callback function.
     * @return A RAII object which when destroyed will unsubscribe the callback.
     */
    subscription subscribe(std::invocable<Args...> auto &&callback) noexcept
    {
        auto sub = subscription{this};
        _callbacks.emplace_back(&sub, tt_forward(callback));
        return sub;
    }

    /** Call the subscribed callbacks with the given arguments.
     * The callbacks will be run from the main thread, asynchronously.
     *
     * @param args The arguments to pass with the invocation of the callback
     */
    void operator()(Args const &...args) const noexcept requires(std::is_same_v<result_type, void>)
    {
        for (auto &callback : _callbacks) {
            callback.second(args...);
        }
    }

    /** Call the subscribed callbacks with the given arguments.
     * The callbacks will be called from the current thread.
     *
     * @param args The arguments to pass with the invocation of the callback
     * @return The result of each callback.
     */
    generator<result_type> operator()(Args const &...args) const noexcept requires(not std::is_same_v<result_type, void>)
    {
        for (auto &callback : _callbacks) {
            co_yield callback.second(args...);
        }
    }

private :
    mutable std::vector<std::pair<subscription const *, callback_type>> _callbacks;

    void unsubscribe(subscription const *sub) noexcept
    {
        ttlet erase_count = std::erase_if(_callbacks, [sub](ttlet &item) {
            return item.first == sub;
        });
        tt_axiom(erase_count == 1);
    }

    void move_subscription(subscription const *sub, subscription const *new_sub) noexcept
    {
        ttlet it = std::find_if(_callbacks.begin(), _callbacks.end(), [sub](ttlet &item) {
            return item.first == sub;
        });
        tt_axiom(it != _callbacks.end());
        it->first = new_sub;
    }

    void copy_subscription(subscription const *sub, subscription const *new_sub) noexcept
    {
        ttlet it = std::find_if(_callbacks.begin(), _callbacks.end(), [sub](ttlet &item) {
            return item.first == sub;
        });
        tt_axiom(it != _callbacks.end());
        _callbacks.emplace_back(new_sub, it->second);
    }
};

template<typename Notifier>
class notifier_awaiter {
public:
    using notifier_type = Notifier;
    using result_type = notifier_type::result_type;
    using handle_type = std::coroutine_handle<>;

    constexpr notifier_awaiter() noexcept : _notifier(nullptr) {}
    constexpr notifier_awaiter(notifier_type &notifier) noexcept : _notifier(&notifier) {}
    constexpr notifier_awaiter(notifier_awaiter const &) noexcept = default;
    constexpr notifier_awaiter(notifier_awaiter &&) noexcept = default;
    constexpr notifier_awaiter &operator=(notifier_awaiter const &) noexcept = default;
    constexpr notifier_awaiter &operator=(notifier_awaiter &&) noexcept = default;

    [[nodiscard]] constexpr bool was_triggered() noexcept
    {
        return _triggered;
    }

    [[nodiscard]] constexpr bool await_ready() noexcept
    {
        return false;
    }

    void await_suspend(handle_type handle) noexcept
    {
        tt_axiom(_notifier != nullptr);
        _triggered = false;

        // We can use the this pointer in the callback, as `await_suspend()` is called by
        // the co-routine on the same object as `await_resume()`.
        _subscription = _notifier->subscribe([handle, this] {
            this->_triggered = true;
            handle.resume();
        });
    }

    constexpr result_type await_resume() noexcept {}

    [[nodiscard]] bool operator==(notifier_awaiter const &rhs) const noexcept
    {
        return _notifier == rhs._notifier;
    }

private:
    using subscription = notifier_type::subscription;

    notifier_type *_notifier;
    subscription _subscription;
    bool _triggered = false;
};

} // namespace tt::inline v1
