// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "unfair_recursive_mutex.hpp"
#include <mutex>
#include <vector>
#include <tuple>
#include <functional>

namespace tt {

template<typename T>
class notifier {
};

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
    static_assert(std::is_same_v<Result,void>, "Result of a notifier must be void.");

    using callback_type = std::function<Result(Args const &...)>;
    using callback_ptr_type = std::shared_ptr<callback_type>;

    /** Add a callback to the notifier.
     * Ownership of the callback belongs with the caller of `subscribe()`. The
     * `notifier` will hold a weak_ptr to the callback so that when the callback is destroyed
     * it will no longer be called.
     *
     * @param callback_ptr A shared_ptr to a callback function.
     */
    void subscribe_ptr(callback_ptr_type const &callback_ptr) noexcept
    {
        auto lock = std::scoped_lock(_mutex);

        ttlet i = std::find_if(_callbacks.cbegin(), _callbacks.cend(), [&callback_ptr](ttlet &item) {
            return item.lock() == callback_ptr;
        });

        if (i == _callbacks.cend()) {
            _callbacks.emplace_back(callback_ptr);
        }
    }

    /** Add a callback to the notifier.
     * Ownership of the callback belongs with the caller of `subscribe()`. The
     * `notifier` will hold a weak_ptr to the callback so that when the callback is destroyed
     * it will no longer be called.
     *
     * @param callback The callback-function to register.
     * @return A shared_ptr to a function object holding the callback.
     */
    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        auto callback_ptr = std::make_shared<callback_type>(std::forward<decltype(callback)>(callback));

        auto lock = std::scoped_lock(_mutex);
        _callbacks.emplace_back(callback_ptr);
        return callback_ptr;
    }

    /** Remove a callback from the notifier.
     * @param callback_ptr A share_ptr to the callback function to unsubscribe.
     */
    void unsubscribe(callback_ptr_type const &callback_ptr) noexcept
    {
        auto lock = std::scoped_lock(_mutex);

        ttlet new_end = std::remove_if(_callbacks.begin(), _callbacks.end(), [&callback_ptr](ttlet &item) {
            return item.expired() || item.lock() == callback_ptr;
        });
        _callbacks.erase(new_end, _callbacks.cend());
    }

    /** Call the subscribed callbacks with the given arguments.
     * @param args The arguments to pass with the invocation of the callback
     */
    void operator()(Args const &...args) const noexcept
    {
        auto lock = std::scoped_lock(_mutex);
        tt_assert(!_executing_callbacks);
        _executing_callbacks = true;

        auto i = _callbacks.begin();
        while (i != _callbacks.end()) {
            if (auto callback = i->lock()) {
                (*callback)(args...);
                ++i;

            } else {
                i = _callbacks.erase(i);
            }
        };
        _executing_callbacks = false;
    }

private:
    mutable unfair_recursive_mutex _mutex;
    mutable bool _executing_callbacks = false;
    mutable std::vector<std::weak_ptr<callback_type>> _callbacks;
};

} // namespace tt
