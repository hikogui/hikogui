// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <functional>
#include "../cpu_utc_clock.hpp"
#include "../unfair_mutex.hpp"
#include "../notifier.hpp"
#include "../required.hpp"

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
    using notifier_type = notifier<void(value_type)>;
    using callback_type = typename notifier_type::callback_type;
    using callback_ptr_type = typename notifier_type::callback_ptr_type;
    using time_point = typename hires_utc_clock::time_point;
    using duration = typename hires_utc_clock::duration;

    observable_base(observable_base const &) = delete;
    observable_base(observable_base &&) = delete;
    observable_base &operator=(observable_base const &) = delete;
    observable_base &operator=(observable_base &&) = delete;
    virtual ~observable_base() = default;

    /** Constructor.
     */
    observable_base() noexcept : _previous_value(), _last_modified(), _notifier() {}

    /** Get the previous value
     */
    [[nodiscard]] value_type previous_value() const noexcept
    {
        ttlet lock = std::scoped_lock(_mutex);
        return _previous_value;
    }

    /** Time when the value was modified last.
     */
    [[nodiscard]] time_point time_when_last_modified() const noexcept
    {
        ttlet lock = std::scoped_lock(_mutex);
        return _last_modified;
    }

    /** Duration since the value was last modified.
     */
    [[nodiscard]] duration duration_since_last_modified() const noexcept
    {
        return cpu_utc_clock::now() - time_when_last_modified();
    }

    /** The relative time since the start of the animation.
     * The relative time since the start of the animation according to the duration of the animation.
     *
     * @param A relative value between 0.0 and 1.0.
     */
    [[nodiscard]] float animation_progress(duration animation_duration) const noexcept
    {
        tt_assume(animation_duration.count() != 0);

        return std::clamp(
            numeric_cast<float>(duration_since_last_modified().count()) / numeric_cast<float>(animation_duration.count()),
            0.0f,
            1.0f);
    }

    /** Get the current value.
     * The value is often calculated directly from the cached values retrieved from
     * notifications down the chain.
     */
    [[nodiscard]] virtual value_type load() const noexcept = 0;

    /** Get the current value animated over the animation_duration.
     */
    [[nodiscard]] value_type load(duration animation_duration) const noexcept
    {
        return mix(animation_progress(animation_duration), previous_value(), load());
    }

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

    /** Notify listeners of a change in value.
     * This function is used to notify listeners of this observable and also
     * to keep track of the previous value and start the animation.
     */
    void notify(value_type const &old_value, value_type const &new_value) noexcept
    {
        {
            ttlet lock = std::scoped_lock(_mutex);
            _previous_value = old_value;
            _last_modified = cpu_utc_clock::now();
        }
        _notifier(new_value);
    }

private:
    value_type _previous_value;
    time_point _last_modified;
    notifier_type _notifier;
};

} // namespace tt::detail