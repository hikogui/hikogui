// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
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

inline std::string format_full_datetime(hires_utc_clock::time_point utc_timestamp, date::time_zone const *time_zone)
{
    let nanoseconds = utc_timestamp.time_since_epoch().count() % 1000000000;

    let sys_timestamp = hires_utc_clock::to_system_time_point(utc_timestamp);
    let local_zoned_time = date::make_zoned(time_zone, sys_timestamp);
    let local_time = local_zoned_time.get_local_time();

    let daypoint = date::floor<date::days>(local_time);

    using CT = typename std::common_type<std::chrono::system_clock::duration, std::chrono::seconds>::type;
    let tod = date::time_of_day<CT>{local_time - date::local_seconds{daypoint}};
    let seconds = tod.seconds().count();

    let local_timestring = date::format("%Y-%m-%d %H:%M", local_zoned_time);
    return fmt::format("{}:{:02}.{:09}", local_timestring, seconds, nanoseconds);
}

}
