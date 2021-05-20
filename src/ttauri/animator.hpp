// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "hires_utc_clock.hpp"
#include "algorithm.hpp"
#include <chrono>

namespace tt {

template<typename T>
class animator {
public:
    using value_type = T;

    animator(hires_utc_clock::duration animation_duration) noexcept : _animation_duration(animation_duration) {}

    void update(value_type new_value, hires_utc_clock::time_point current_time) noexcept
    {
        if (new_value != _new_value) {
            _old_value = _new_value;
            _new_value = new_value;
            _start_time = current_time;
        }
        _current_time = current_time;
    }

    [[nodiscard]] bool is_animating() const noexcept
    {
        return progress() < 1.0f;
    }

    value_type current_value() const noexcept {
        return mix(_old_value, _new_value, progress());
    }

private:
    value_type _old_value;
    value_type _new_value;
    hires_utc_clock::time_point _start_time;
    hires_utc_clock::time_point _current_time;
    hires_utc_clock::duration _animation_duration;

    float progress() const noexcept
    {
        auto dt = _current_time - _start_time;
        return static_cast<float>(dt / 1ms) / static_cast<float>(_animation_duration / 1ms);
    }
};

} // namespace tt
