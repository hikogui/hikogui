// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <functional>
#include "../cpu_utc_clock.hpp"
#include "../fast_mutex.hpp"
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
    using callback_type = std::function<void(T const &)>;
    using time_point = typename hires_utc_clock::time_point;
    using duration = typename hires_utc_clock::duration;

protected:
    mutable fast_mutex mutex;

private:
    T _previous_value;
    time_point _last_modified;
    notifier<T> notifier;

public:
    observable_base(observable_base const &) = delete;
    observable_base(observable_base &&) = delete;
    observable_base &operator=(observable_base const &) = delete;
    observable_base &operator=(observable_base &&) = delete;
    virtual ~observable_base() = default;

    /** Constructor.
    */
    observable_base() noexcept :
        _previous_value(), _last_modified(), notifier() {}

    /** Get the previous value
    */
    [[nodiscard]] T previous_value() const noexcept {
        ttlet lock = std::scoped_lock(mutex);
        return _previous_value;
    }

    /** Time when the value was modified last.
    */
    [[nodiscard]] time_point time_when_last_modified() const noexcept {
        ttlet lock = std::scoped_lock(mutex);
        return _last_modified;
    }

    /** Duration since the value was last modified.
    */
    [[nodiscard]] duration duration_since_last_modified() const noexcept {
        return cpu_utc_clock::now() - time_when_last_modified();
    }

    /** The relative time since the start of the animation.
    * The relative time since the start of the animation according to the duration of the animation.
    *
    * @param A relative value between 0.0 and 1.0.
    */
    [[nodiscard]] float animation_progress(duration animation_duration) const noexcept {
        tt_assume(animation_duration.count() != 0);

        return std::clamp(
            numeric_cast<float>(duration_since_last_modified().count()) / numeric_cast<float>(animation_duration.count()),
            0.0f,
            1.0f
        );
    }

    /** Get the current value.
    * The value is often calculated directly from the cached values retrieved from
    * notifications down the chain.
    */
    [[nodiscard]] virtual T load() const noexcept = 0;

    /** Get the current value animated over the animation_duration.
    */
    [[nodiscard]] T load(duration animation_duration) const noexcept {
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
    virtual bool store(T const &new_value) noexcept = 0;

    /** Add a callback as a listener.
    * @param callback The callback to register, the new value is passed as the first argument.
    * @return The id of the callback, used to unregister the callback.
    */
    [[nodiscard]] size_t add_callback(callback_type callback) noexcept {
        return notifier.add(callback);
    }

    /** Remove a callback.
    * @param id The id of the callback returned by the `add_callback()` method.
    */
    void remove_callback(size_t id) noexcept {
        notifier.remove(id);
    }

protected:

    /** Notify listeners of a change in value.
    * This function is used to notify listeners of this observable and also
    * to keep track of the previous value and start the animation.
    */
    void notify(T const &old_value, T const &new_value) noexcept {
        {
            ttlet lock = std::scoped_lock(mutex);
            _previous_value = old_value;
            _last_modified = cpu_utc_clock::now();
        }
        notifier(new_value);
    }
};

}