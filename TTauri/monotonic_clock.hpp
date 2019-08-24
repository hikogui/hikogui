// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/required.hpp"
#include <atomic>
#include <chrono>

namespace TTauri {

template<typename C>
struct monotonic_clock {
    using clock = C;

    using rep = typename clock::rep;
    using period = typename clock::period;
    using duration = typename clock::duration;
    using timepoint = typename clock::timepoint;
    static const bool is_steady = true;

    inline static std::atomic<rep> value;

    /*! Get the current time/unique id.
     * This function is wait-free, as long as the depended clock is also wait-free..
     */
    static timepoint now() {
        let new_value = C::now().duration().count();
        auto old_value = value.load();

        // Try to update the value if the time has a higher value.
        // due to contention we could fail, but we will not retry
        // instead we increment the current value.
        if (new_value > old_value) {
            if (value.compare_exchange_strong(old_value, new_value)) {
                return timepoint(duration(new_value));
            }
        }

        return timepoint(duration(value.fetch_add(1) + 1));
    }

}

}
