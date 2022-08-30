// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "tree.hpp"
#include "notifier.hpp"
#include <memory>
#include <vector>
#include <string>

namespace hi::inline v1 {

/** An abstract observable object.
 *
 * This type is referenced by `observer`s
 */
class observable : public std::enable_shared_from_this<observable> {
public:
    /** The type of the notifier used to notify changes to the value of the observable.
     */
    using notifier_type = notifier<void(void const *)>;

    /** The token returned by `subscribe()`.
     */
    using token_type = notifier_type::token_type;

    /** The type of the callback that can be subscribed.
     */
    using function_proto = notifier_type::function_proto;

    /** The type of the path used for notifying observers.
     */
    using path_type = std::vector<std::string>;

    constexpr virtual ~observable() = default;
    observable(observable const&) = delete;
    observable(observable&&) = delete;
    observable& operator=(observable const&) = delete;
    observable& operator=(observable&&) = delete;
    constexpr observable() noexcept = default;

    /** Get a pointer to the current value.
     *
     * @note `read()` does not `read_lock()` the observable and should be done before `read()`.
     * @return A const pointer to the value. The `observer` should cast this to a pointer to the value-type.
     */
    [[nodiscard]] virtual void const *read() const noexcept = 0;

    /** Allocate and make a copy of the value.
     *
     * @note `copy()` does not `write_lock()` the observable and should be done before `read()`.
     * @param A pointer to the value that was `read()`.
     * @return A pointer to a newly allocated copy of the value.
     */
    [[nodiscard]] virtual void *copy(void const *ptr) const noexcept = 0;

    /** Commit the modified copy.
     *
     * @note `commit()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void commit(void *ptr) noexcept = 0;

    /** Abort the modified copy.
     *
     * @note `abort()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void abort(void *ptr) const noexcept = 0;

    /** Lock for reading.
     */
    virtual void read_lock() const noexcept = 0;

    /** Unlock for reading.
     */
    virtual void read_unlock() const noexcept = 0;

    /** Lock for writing.
     */
    virtual void write_lock() const noexcept = 0;

    /** Unlock for writing.
     */
    virtual void write_unlock() const noexcept = 0;

    /** Subscribe a callback with the observer.
     *
     * @param path The path within the observable-value that being watched.
     * @param flags The way the callback should be called.
     * @param function The function to be called when the watched observable-value changes.
     *                 The function has two `void *` arguments to the old value and the new value.
     *                 It is the task of the observer to cast the `void *` to the actual value-type.
     * @return A token which will extend the lifetime of the function. When the token is destroyed
     *         the function will be unsubscribed.
     */
    [[nodiscard]] token_type
    subscribe(path_type const& path, callback_flags flags, forward_of<function_proto> auto&& function) noexcept
    {
        auto& notifier = _notifiers[path];
        return notifier.subscribe(flags, hi_forward(function));
    }

    /** Called by a observer to notify all observers that the value has changed.
     *
     * The @a path argument is used to determine which of the subscribed callback will be called.
     *  - All callbacks which are a prefix of @a path.
     *  - All callbacks which have @a path as a prefix.
     *
     * @param ptr The pointer to the value.
     * @param path The path of the observed-value that was modified.
     */
    void notify(void const *ptr, path_type const& path) const noexcept
    {
        _notifiers.walk_including_path(path, [ptr](notifier_type const& notifier) {
            notifier(ptr);
        });
    }

private:
    tree<std::string, notifier_type> _notifiers;
};

} // namespace hi::inline v1
