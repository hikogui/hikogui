// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <chrono>

namespace TTauri {

/*! Timestamp
 */
struct hires_utc_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<hires_utc_clock>;
    static const bool is_steady = false;

	static time_point now();

    std::chrono::system_clock::time_point convert_to_system_clock(TTauri::hires_utc_clock::time_point x) {
        if constexpr (std::is_same_v<std::chrono::system_clock::period, std::nano>) {
            return std::chrono::system_clock::time_point{
                std::chrono::system_clock::duration(x.time_since_epoch().count())
            };
        } else if constexpr (std::is_same_v<std::chrono::system_clock::period, std::micro>) {
            return std::chrono::system_clock::time_point{
                std::chrono::system_clock::duration(x.time_since_epoch().count() / 1000)
            };
        } else {
            no_default;
        }
    }

    static std::string local_time_string(time_point timestamp) {
        return "2000-01-01 12:23:34.000000000";
    }
};

}
