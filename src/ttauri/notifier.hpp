// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <mutex>
#include <vector>
#include <tuple>
#include <functional>

namespace tt {

/** A notifier which can be used to call a set of registred callbacks.
 * This class is thread-safe; however you must not use this object
 * from within the callback.
 *
 * @tparam Args The argument types of the callback function.
 */
template<typename... Args>
class notifier {
public:
    using callback_type = std::function<void(Args const &...)>;

private:
    mutable std::recursive_mutex mutex;
    mutable bool executing_callbacks = false;
    size_t counter = 0;
    std::vector<std::pair<size_t,callback_type>> callbacks;

public:
    /** Add a callback to the notifier.
     * @param callback The callback to register.
     * @return An id used to unregister a callback from the notifier.
     */
    size_t add(callback_type callback) noexcept {
        auto lock = std::scoped_lock(mutex);
        tt_assert(!executing_callbacks);

        auto id = ++counter;
        callbacks.emplace_back(id, std::move(callback));
        return id;
    }

    /** Add a callback to the notifier and invoke it.
    * @param callback The callback to register.
    * @param args The arguments to pass with the invocation of the callback
    * @return An id used to unregister a callback from the notifier.
    */
    size_t add_and_call(callback_type callback, Args const &... args) noexcept {
        auto lock = std::scoped_lock(mutex);

        ttlet id = add(callback);

        executing_callbacks = true;
        callback(args...);
        executing_callbacks = false;

        return id;
    }

    /** Remove a callback from the notifier.
     * @param id The id returned from `add()` and `add_and_call()`.
     */
    void remove(size_t id) noexcept {
        auto lock = std::scoped_lock(mutex);
        tt_assert(!executing_callbacks);

        ttlet new_end = std::remove_if(callbacks.begin(), callbacks.end(), [id](ttlet &item) {
            return item.first == id;
        });
        callbacks.erase(new_end, callbacks.cend());
    }

    /** Call the registerd callbacks with the given arguments.
     * @param args The arguments to pass with the invocation of the callback
     */
    void operator()(Args const &... args) const noexcept {
        auto lock = std::scoped_lock(mutex);
        tt_assert(!executing_callbacks);

        executing_callbacks = true;
        for (ttlet &[id, callback] : callbacks) {
            callback(args...);
        }
        executing_callbacks = false;
    }

};

}