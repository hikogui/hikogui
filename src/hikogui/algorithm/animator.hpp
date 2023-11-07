// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "../utility/utility.hpp"
#include "../time/time.hpp"
#include <cmath>
#include <chrono>
#include <algorithm>

hi_export_module(hikogui.algorithm.animator);

hi_export namespace hi::inline v1 {

enum class animator_state {
    uninitialized, idle, running, end
};

/** A type that gets animated between two values.
 */
template<arithmetic T>
class animator {
public:
    using value_type = T;

    /** Constructor.
     * @param animation_duration The duration to animate from start to end value.
     */
    animator(std::chrono::nanoseconds animation_duration) noexcept :
        _animation_duration(animation_duration), _old_value(), _new_value(), _start_time(), _current_time(std::chrono::utc_clock::now()), _state(animator_state::uninitialized)
    {
    }

    /** Update the value and time.
     * @param new_value The value to animate toward.
     * @param current_time The current time.
     * @return current animation state (idle, running or end).
     */
    [[nodiscard]] animator_state update(value_type new_value, utc_nanoseconds current_time) noexcept
    {
        if (_state == animator_state::uninitialized) {
            _state = animator_state::idle;
            _old_value = new_value;
            _new_value = new_value;
            _start_time = {};
        } else if (new_value != _new_value) {
            _state = animator_state::running;
            _old_value = _new_value;
            _new_value = new_value;
            _start_time = current_time;
        }
        _current_time = current_time;
        return is_animating();
    }

    /** Check if the animation is currently running.
     * @return current animation state (idle, running or end).
     */
    [[nodiscard]] animator_state is_animating() const noexcept
    {
        switch (_state) {
        case animator_state::uninitialized:
            return animator_state::uninitialized;
        case animator_state::idle:
            return animator_state::idle;
        case animator_state::running:
            if (progress() < 1.0f) {
                return animator_state::running;
            } else {
                return _state = animator_state::end;
            }
        case animator_state::end:
            return _state = animator_state::idle;
        default:
            hi_no_default();
        }
    }

    /** The interpolated value between start and end value.
     */
    value_type current_value() const noexcept
    {
        return std::lerp(_old_value, _new_value, progress());
    }

private:
    std::chrono::nanoseconds _animation_duration;

    value_type _old_value;
    value_type _new_value;
    utc_nanoseconds _start_time;
    utc_nanoseconds _current_time;
    mutable animator_state _state;

    float progress() const noexcept
    {
        using namespace std::chrono_literals;

        hilet dt = _current_time - _start_time;
        hilet p = static_cast<float>(dt / 1ms) / static_cast<float>(_animation_duration / 1ms);
        return std::clamp(p, 0.0f, 1.0f);
    }
};

} // namespace hi::inline v1
