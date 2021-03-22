// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include <date/tz.h>
#include <chrono>
#include <type_traits>

namespace tt {

/** Timestamp
 */
struct hires_utc_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<hires_utc_clock>;
    static const bool is_steady = false;

	static time_point now() noexcept;

};

std::string format_engineering(hires_utc_clock::duration duration);

/** Return a ISO-8601 formated date-time.
 * @param utc_timestamp The time_point to format.
 */
std::string format_iso8601_utc(hires_utc_clock::time_point utc_timestamp) noexcept;

/** Return a ISO-8601 formated date-time.
 * @param utc_timestamp The time_point to format.
 * @param time_zone If time_zone is a nullptr then the current timezone is used.
 */
std::string format_iso8601(hires_utc_clock::time_point utc_timestamp, date::time_zone const *time_zone = nullptr) noexcept;

}
