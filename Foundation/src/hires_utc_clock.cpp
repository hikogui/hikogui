// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/Foundation/globals.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>

namespace TTauri {

using namespace std::chrono_literals;

std::string format_engineering(hires_utc_clock::duration duration)
{
    if (duration >= 1s) {
        return fmt::format("{:.3g} s", static_cast<double>(duration / 1ns) / 1'000'000'000);
    } else if (duration >= 1ms) {
        return fmt::format("{:.3g} ms", static_cast<double>(duration / 1ns) / 1'000'000);
    } else if (duration >= 1us) {
        return fmt::format("{:.3g} us", static_cast<double>(duration / 1ns) / 1'000);
    } else {
        return fmt::format("{:.3g} ns", static_cast<double>(duration / 1ns));
    }
}

std::string format_iso8601_utc(hires_utc_clock::time_point utc_timestamp)
{
    let nanoseconds = utc_timestamp.time_since_epoch().count() % 1000000000;

    let sys_timestamp = hires_utc_clock::to_system_time_point(utc_timestamp);

    let daypoint = date::floor<date::days>(sys_timestamp);
    let ymd = date::year_month_day{daypoint};

    let tod = date::make_time(sys_timestamp - daypoint);

    return fmt::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:09}Z",
        static_cast<int>(ymd.year()),
        static_cast<unsigned>(ymd.month()),
        static_cast<unsigned>(ymd.day()),
        tod.hours().count(), tod.minutes().count(), tod.seconds().count(),
        nanoseconds
    );
}

std::string format_iso8601(hires_utc_clock::time_point utc_timestamp, date::time_zone const *time_zone)
{
    if (time_zone == nullptr) {
        time_zone = timeZone;
    }

    if (time_zone == nullptr) {
        return format_iso8601_utc(utc_timestamp);
    }

    let nanoseconds = utc_timestamp.time_since_epoch().count() % 1000000000;

    let sys_timestamp = hires_utc_clock::to_system_time_point(utc_timestamp);

    try {
        let local_zoned_time = date::make_zoned(time_zone, sys_timestamp);
        let local_time = local_zoned_time.get_local_time();

        let daypoint = date::floor<date::days>(local_time);

        using CT = typename std::common_type<std::chrono::system_clock::duration, std::chrono::seconds>::type;
        let tod = date::time_of_day<CT>{local_time - date::local_seconds{daypoint}};
        let seconds = tod.seconds().count();

        let tz_offset = local_zoned_time.get_info().offset;

        std::string tz_offset_string;
        if (tz_offset == 0min) {
            tz_offset_string = "Z";
        } else if (tz_offset >= 0min) {
            let tz_offset_minutes = (tz_offset % 1h).count();
            let tz_offset_hours = tz_offset / 1h;
            if (tz_offset_minutes == 0) {
                tz_offset_string = fmt::format("+{:02}", tz_offset_hours);
            } else {
                tz_offset_string = fmt::format("+{:02}{:02}", tz_offset_hours, tz_offset_minutes);
            }
        } else {
            let tz_offset_minutes = ((-tz_offset) % 1h).count();
            let tz_offset_hours = (-tz_offset) / 1h;
            if (tz_offset_minutes == 0) {
                tz_offset_string = fmt::format("-{:02}", tz_offset_hours);
            } else {
                tz_offset_string = fmt::format("-{:02}{:02}", tz_offset_hours, tz_offset_minutes);
            }
        }

        let local_timestring = date::format("%Y-%m-%dT%H:%M", local_zoned_time);

        return fmt::format("{}:{:02}.{:09}{}", local_timestring, seconds, nanoseconds, tz_offset_string);

    } catch (...) {
        return format_iso8601_utc(utc_timestamp);
    }
}

}