
#pragma once

#include "TTauri/Foundation/cpu_utc_clock.hpp"

namespace TTauri {

template<typename T, int64_t AnimationDurationNS typename Clock=hires_utc_clock>
class AnimatedValue {
    using value_type = T;
    using duration = Clock::duration;
    using time_point = Clock::time_point;

    constexpr duration animationDuration = DurationNS * 1ns;

    value_type currentValue;
    value_type previousValue;
    time_point changeTimePoint;

public:
    value_type value(time_point tp);

    setValue(value_type const &value, time_point tp) {
        previousValue = currentValue;
        currentValue = value;
        changeTimePoint = tp;
    }

};

}
