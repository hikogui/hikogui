// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "hires_utc_clock.hpp"
#include "algorithm.hpp"
#include <chrono>
#include <cmath>
#include "concepts.hpp"

namespace tt {

/** A type that gets animated between two values.
 */
template<arithmetic T>
class animator {
public:
    using value_type = T;

    /** Constructor.
     * @param animation_duration The duration to animate from start to end value.
     */
    animator(hires_utc_clock::duration animation_duration) noexcept : _animation_duration(animation_duration) {}

    /** Update the value and time.
     * @param new_value The value to animate toward.
     * @param current_time The current time.
     */
    void update(value_type new_value, hires_utc_clock::time_point current_time) noexcept
    {
        if (not initialized) {
            initialized = true;
            _old_value = new_value;
            _new_value = new_value;
            _start_time = current_time;

        } else if (new_value != _new_value) {
            _old_value = _new_value;
            _new_value = new_value;
            _start_time = current_time;
        }
        _current_time = current_time;
    }

    /** Check if the animation is currently running.
     */
    [[nodiscard]] bool is_animating() const noexcept
    {
        tt_axiom(initialized);
        return progress() < 1.0f;
    }

    /** The interpolated value between start and end value.
     */
    value_type current_value() const noexcept {
        tt_axiom(initialized);
        return std::lerp(_old_value, _new_value, progress());
    }

private:
    value_type _old_value;
    value_type _new_value;
    hires_utc_clock::time_point _start_time;
    hires_utc_clock::time_point _current_time;
    hires_utc_clock::duration _animation_duration;
    bool initialized = false;

    float progress() const noexcept
    {
        using namespace std::literals::chrono_literals;

        ttlet dt = _current_time - _start_time;
        ttlet p = static_cast<float>(dt / 1ms) / static_cast<float>(_animation_duration / 1ms);
        return std::clamp(p, 0.0f, 1.0f);
    }
};

} // namespace tt
