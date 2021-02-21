// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_utc_clock.hpp"
#include "application.hpp"
#include "logger.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>

namespace tt {

using namespace std::chrono_literals;

std::string format_engineering(hires_utc_clock::duration duration)
{
    if (duration >= 1s) {
        return fmt::format("{:.3g} s ", static_cast<double>(duration / 1ns) / 1'000'000'000);
    } else if (duration >= 1ms) {
        return fmt::format("{:.3g} ms", static_cast<double>(duration / 1ns) / 1'000'000);
    } else if (duration >= 1us) {
        return fmt::format("{:.3g} us", static_cast<double>(duration / 1ns) / 1'000);
    } else {
        return fmt::format("{:.3g} ns", static_cast<double>(duration / 1ns));
    }
}

std::string format_iso8601_utc(hires_utc_clock::time_point utc_timestamp) noexcept
{
    ttlet nanoseconds = utc_timestamp.time_since_epoch().count() % 1000000000;

    ttlet sys_timestamp = hires_utc_clock::to_system_time_point(utc_timestamp);

    ttlet daypoint = date::floor<date::days>(sys_timestamp);
    ttlet ymd = date::year_month_day{daypoint};

    ttlet tod = date::make_time(sys_timestamp - daypoint);

    return fmt::format(
        "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:09}Z",
        static_cast<int>(ymd.year()),
        static_cast<unsigned>(ymd.month()),
        static_cast<unsigned>(ymd.day()),
        tod.hours().count(),
        tod.minutes().count(),
        tod.seconds().count(),
        nanoseconds);
}

std::string format_iso8601(hires_utc_clock::time_point utc_timestamp, date::time_zone const *time_zone) noexcept
{
    if (time_zone == nullptr) {
        try {
            time_zone = date::current_zone();
            tt_axiom(time_zone != nullptr);

        } catch (std::runtime_error const &) {
            return format_iso8601_utc(utc_timestamp);
        }
    }

    ttlet nanoseconds = utc_timestamp.time_since_epoch().count() % 1000000000;

    ttlet sys_timestamp = hires_utc_clock::to_system_time_point(utc_timestamp);

    try {
        ttlet local_zoned_time = date::make_zoned(time_zone, sys_timestamp);
        ttlet local_time = local_zoned_time.get_local_time();

        ttlet daypoint = date::floor<date::days>(local_time);

        using CT = typename std::common_type<std::chrono::system_clock::duration, std::chrono::seconds>::type;
        ttlet tod = date::time_of_day<CT>{local_time - date::local_seconds{daypoint}};
        ttlet seconds = tod.seconds().count();

        ttlet tz_offset = local_zoned_time.get_info().offset;

        std::string tz_offset_string;
        if (tz_offset == 0min) {
            tz_offset_string = "Z";
        } else if (tz_offset >= 0min) {
            ttlet tz_offset_minutes = (tz_offset % 1h).count();
            ttlet tz_offset_hours = tz_offset / 1h;
            if (tz_offset_minutes == 0) {
                tz_offset_string = fmt::format("+{:02}", tz_offset_hours);
            } else {
                tz_offset_string = fmt::format("+{:02}{:02}", tz_offset_hours, tz_offset_minutes);
            }
        } else {
            ttlet tz_offset_minutes = ((-tz_offset) % 1h).count();
            ttlet tz_offset_hours = (-tz_offset) / 1h;
            if (tz_offset_minutes == 0) {
                tz_offset_string = fmt::format("-{:02}", tz_offset_hours);
            } else {
                tz_offset_string = fmt::format("-{:02}{:02}", tz_offset_hours, tz_offset_minutes);
            }
        }

        ttlet local_timestring = date::format("%Y-%m-%dT%H:%M", local_zoned_time);

        return fmt::format("{}:{:02}.{:09}{}", local_timestring, seconds, nanoseconds, tz_offset_string);

    } catch (...) {
        return format_iso8601_utc(utc_timestamp);
    }
}

} // namespace tt