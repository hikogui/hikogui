// Copyright 2020 Pokitec
// All rights reserved.

#pragma once
#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/Foundation/math.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include <mutex>
#include <algorithm>
#include <optional>

namespace tt {

template<typename T>
class animated {
public:
    using observed_type = T;
    using value_type = make_value_type_t<observed_type>;
    using mix_type = std::conditional_t<std::is_same_v<value_type,bool>, double, value_type>;
    using clock_type = hires_utc_clock;
    using duration = typename clock_type::duration;
    using time_point = typename clock_type::time_point;

private:
    mutable std::mutex mutex;

    observed_type observed_value;

    /** Value at the start of the animation.
    */
    value_type prev_value;

    /** Value at the end of the animation.
    */
    value_type next_value;

    /** Time for the animation to complete.
    */
    duration animation_duration;

    /** Time point when the current animation was started.
    */
    time_point current_time_point;

public:
    animated(animated const &) = delete;
    animated(animated &&) = delete;
    animated &operator=(animated const &) = delete;
    animated &operator=(animated &&) = delete;

    template<typename... Args>
    animated(duration animation_duration, Args &&... args) noexcept :
        observed_value(std::forward<Args>(args)...),
        animation_duration(animation_duration), current_time_point()
    {
        prev_value = observed_value;
        next_value = prev_value;
    }

    template<typename Func>
    size_t add_callback(Func &&func) {
        return observed_value.add_callback(std::forward<Func>(func));
    }

    operator value_type () const noexcept {
        return static_cast<value_type>(observed_value);
    }

    animated &operator=(value_type const &rhs) noexcept {
        observed_value = rhs;
        return *this;
    }

    animated &operator=(value_type &&rhs) noexcept {
        observed_value = std::move(rhs);
        return *this;
    }

    /** Animation tick will animate the changes in the observed value.
    *
    */
    std::tuple<double,mix_type> animation_tick(time_point tp) noexcept {
        auto lock = std::scoped_lock(mutex);

        auto duration_since_start = tp - current_time_point;
        auto animation_progress =
            numeric_cast<double>(duration_since_start.count()) /
            numeric_cast<double>(animation_duration.count());

        ttlet prev_value_ = static_cast<mix_type>(prev_value);
        ttlet next_value_ = static_cast<mix_type>(next_value);
        ttlet mix_value = mix(animation_progress, prev_value_, next_value_);

        ttlet new_value = static_cast<value_type>(observed_value);
        if (new_value != next_value) {
            if constexpr (std::is_same_v<value_type, bool>) {
                prev_value = next_value;
            } else {
                prev_value = static_cast<value_type>(mix_value);
            }
            next_value = new_value;
            current_time_point = tp;

            animation_progress = 0.0;

        } else {
            animation_progress = std::clamp(animation_progress, 0.0, 1.0);
        }
        
        return {animation_progress, mix_value};
    }
};

}
