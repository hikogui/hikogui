// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>
#include "../unfair_mutex.hpp"
#include "../notifier.hpp"
#include "../required.hpp"
#include "../hires_utc_clock.hpp"

namespace tt::detail {

/** Observable abstract base class.
 * Objects of the observable_base class will notify listeners through
 * callbacks of changes of its value.
 *
 * This class does not hold the value itself, concrete subclasses will
 * either hold the value or calculate the value on demand. In many cases
 * concrete subclasses may be sub-expressions of other observable objects.
 *
 * This object will also track the time when the value was last modified
 * so that the value can be animated. Usefull when displaying the value
 * as an animated graphic element. For calculating inbetween values
 * it will keep track of the previous value.
 *
 */
template<typename T>
class observable_base {
public:
    using value_type = T;
    using notifier_type = notifier<void()>;
    using callback_type = typename notifier_type::callback_type;
    using callback_ptr_type = typename notifier_type::callback_ptr_type;

    observable_base(observable_base const &) = delete;
    observable_base(observable_base &&) = delete;
    observable_base &operator=(observable_base const &) = delete;
    observable_base &operator=(observable_base &&) = delete;
    virtual ~observable_base() = default;

    /** Constructor.
     */
    observable_base() noexcept : _notifier() {}

    /** Get the current value.
     * The value is often calculated directly from the cached values retrieved from
     * notifications down the chain.
     */
    [[nodiscard]] virtual value_type load() const noexcept = 0;

    /** Set the value.
     * The value is often not directly stored, but instead forwarded up
     * the chain of observables. And then let the notifications flowing backward
     * updated the cached values so that loads() will be quick.
     *
     * @param new_value The new value
     * @return true if the value was different from before.
     */
    virtual bool store(value_type const &new_value) noexcept = 0;

    /** Add a callback as a listener.
     * @param callback The callback to register, the new value is passed as the first argument.
     * @return The shared callback pointer, used to unsubscribe the callback.
     */
    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        return _notifier.subscribe(std::forward<Callback>(callback));
    }

    /** Remove a callback.
     * @param callback_ptr The shared callback pointer returned by the `add_callback()` method.
     */
    void unsubscribe(callback_ptr_type callback_ptr) noexcept
    {
        _notifier.unsubscribe(callback_ptr);
    }

protected:
    mutable unfair_mutex _mutex;
    notifier_type _notifier;
};

} // namespace tt::detail
