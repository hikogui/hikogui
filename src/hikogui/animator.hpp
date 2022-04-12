// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "chrono.hpp"
#include "algorithm.hpp"
#include "concepts.hpp"
#include <cmath>

namespace hi::inline v1 {

/** A type that gets animated between two values.
 */
template<arithmetic T>
class animator {
public:
    using value_type = T;

    /** Constructor.
     * @param animation_duration The duration to animate from start to end value.
     */
    animator(std::chrono::nanoseconds animation_duration) noexcept : _animation_duration(animation_duration) {}

    /** Update the value and time.
     * @param new_value The value to animate toward.
     * @param current_time The current time.
     * @return is_animating
     */
    bool update(value_type new_value, utc_nanoseconds current_time) noexcept
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
        return is_animating();
    }

    /** Check if the animation is currently running.
     */
    [[nodiscard]] bool is_animating() const noexcept
    {
        hi_axiom(initialized);
        return progress() < 1.0f;
    }

    /** The interpolated value between start and end value.
     */
    value_type current_value() const noexcept
    {
        hi_axiom(initialized);
        return std::lerp(_old_value, _new_value, progress());
    }

private:
    value_type _old_value;
    value_type _new_value;
    utc_nanoseconds _start_time;
    utc_nanoseconds _current_time;
    std::chrono::nanoseconds _animation_duration;
    bool initialized = false;

    float progress() const noexcept
    {
        using namespace std::chrono_literals;

        hilet dt = _current_time - _start_time;
        hilet p = static_cast<float>(dt / 1ms) / static_cast<float>(_animation_duration / 1ms);
        return std::clamp(p, 0.0f, 1.0f);
    }
};

} // namespace hi::inline v1
