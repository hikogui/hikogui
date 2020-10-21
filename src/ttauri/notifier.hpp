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

/** A notifier which can be used to call a set of registered callbacks.
 * This class is thread-safe; however you must not use this object
 * from within the callback.
 *
 * @tparam Args The argument types of the callback function.
 */
template<typename... Args>
class notifier {
public:
    using callback_type = std::function<void(Args const &...)>;
    using callback_id_type = size_t;

    /** Add a callback to the notifier.
     * @param callback The callback to register.
     * @return An id used to unregister a callback from the notifier.
     */
    callback_id_type subscribe(callback_type callback) noexcept
    {
        auto lock = std::scoped_lock(_mutex);
        tt_assert(!_executing_callbacks);

        auto id = ++_counter;
        _callbacks.emplace_back(id, std::move(callback));
        return id;
    }

    /** Add a callback to the notifier and invoke it.
     * @param callback The callback to register.
     * @param args The arguments to pass with the invocation of the callback
     * @return An id used to unregister a callback from the notifier.
     */
    callback_id_type add_and_call(callback_type callback, Args const &... args) noexcept
    {
        auto lock = std::scoped_lock(_mutex);

        ttlet id = subscribe(callback);

        _executing_callbacks = true;
        callback(args...);
        _executing_callbacks = false;

        return id;
    }

    /** Remove a callback from the notifier.
     * @param id The id returned from `add()` and `add_and_call()`.
     */
    void unsubscribe(callback_id_type id) noexcept
    {
        auto lock = std::scoped_lock(_mutex);
        tt_assert(!_executing_callbacks);

        ttlet new_end = std::remove_if(_callbacks.begin(), _callbacks.end(), [id](ttlet &item) {
            return item.first == id;
        });
        _callbacks.erase(new_end, _callbacks.cend());
    }

    /** Call the registerd callbacks with the given arguments.
     * @param args The arguments to pass with the invocation of the callback
     */
    void operator()(Args const &... args) const noexcept
    {
        auto lock = std::scoped_lock(_mutex);
        tt_assert(!_executing_callbacks);

        _executing_callbacks = true;
        for (ttlet & [ id, callback ] : _callbacks) {
            callback(args...);
        }
        _executing_callbacks = false;
    }

private:
    mutable unfair_recursive_mutex _mutex;
    mutable bool _executing_callbacks = false;
    callback_id_type _counter = 0;
    std::vector<std::pair<callback_id_type, callback_type>> _callbacks;
};

template<typename Notifier>
class scoped_callback {
public:
    using notifier_type = Notifier;
    using callback_type = typename notifier_type::callback_type;
    using callback_id_type = typename notifier_type::callback_id_type;

    constexpr scoped_callback() noexcept : _notifier(nullptr), _id() {}

    scoped_callback(scoped_callback const &other) = delete;
    scoped_callback &operator=(scoped_callback const &other) = delete;

    constexpr scoped_callback(scoped_callback &&other) noexcept :
        _notifier(other._notifier), _id(other._id)
    {
        other._notifier = nullptr;
    }

    constexpr scoped_callback &operator=(scoped_callback &&other) noexcept {
        _notifier = std::exchange(other._notifier, nullptr);
        _id = other._id;
        return *this;
    }

    ~scoped_callback()
    {
        if (_notifier != nullptr) {
            _notifier->unsubscribe(_id);
        }
    }

    [[nodiscard]] scoped_callback(notifier_type &notifier, callback_type &&callback) noexcept :
        _notifier(&notifier), _id(notifier.subscribe(std::move(callback)))
    {
    }

    [[nodiscard]] scoped_callback(notifier_type &notifier, callback_type const &callback) noexcept :
        _notifier(&notifier), _id(notifier.subscribe(callback))
    {
    }

private:
    callback_id_type _id;
    notifier_type *_notifier;
};

} // namespace tt