// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <date/tz.h>
#include <chrono>
#include <type_traits>

namespace TTauri {

/*! Timestamp
 */
struct hires_utc_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<hires_utc_clock>;
    static const bool is_steady = false;

	static time_point now() noexcept;

    static std::chrono::system_clock::time_point to_system_time_point(time_point x) noexcept {
        static_assert(std::chrono::system_clock::period::num == 1, "Percission of system clock must be 1 second or better.");
        static_assert(std::chrono::system_clock::period::den <= 1000000000, "Percission of system clock must be 1ns or worse.");

        constexpr int64_t nano_to_sys_ratio = 1000000000LL / std::chrono::system_clock::period::den;

        return std::chrono::system_clock::time_point{
            std::chrono::system_clock::duration(x.time_since_epoch().count() / nano_to_sys_ratio)
        };
    }
};

/*! Return a string representation of the time_point as a "year-month-day hour:minute:second.nanoseconds".
 * \param time_zone If time_zone is a nullptr then UTC is used.
 */
std::string format_full_datetime(hires_utc_clock::time_point utc_timestamp, date::time_zone const *time_zone=nullptr);

}
