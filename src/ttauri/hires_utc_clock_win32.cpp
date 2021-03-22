// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_utc_clock.hpp"
#include "subsystem.hpp"
#include "logger.hpp"
#include <Windows.h>
#include <timezoneapi.h>

namespace tt {

hires_utc_clock::time_point hires_utc_clock::now() noexcept
{
    FILETIME ts;
    GetSystemTimePreciseAsFileTime(&ts);

    auto utc_ts = static_cast<int64_t>(ts.dwHighDateTime) << 32;
    utc_ts |= static_cast<int64_t>(ts.dwLowDateTime);

    // Convert to UNIX Epoch. Currently utc_ts is still in 100ns format.
    // Convert  1601-01-1 00:00:00 -> 1970-01-01 00:00:00
    utc_ts -= 11644473600'000'000'0;

    // Convert to 1ns format.
    utc_ts *= 100;

    // Windows time is 37 seconds behind TAI
    utc_ts += 37'000'000'000;

    return time_point(duration(utc_ts));
}

std::string format_iso8601_utc(hires_utc_clock::time_point utc_timestamp) noexcept
{
    ttlet ns = utc_timestamp.time_since_epoch().count();

    ttlet file_time = ((ns - 37'000'000'000) / 100) + 11644473600'000'000'0;
    FILETIME file_time_;
    file_time_.dwLowDateTime = narrow_cast<DWORD>(file_time & 0xffff'ffff);
    file_time_.dwHighDateTime = narrow_cast<DWORD>(file_time >> 32);

    SYSTEMTIME system_time;
    if (!FileTimeToSystemTime(&file_time_, &system_time)) {
        tt_log_error("FileTimeToSystemTime failed {}", get_last_error_message());
        return "<unknown time>";
    }

    ttlet nanoseconds = ns % 1'000'000'000;

    return fmt::format(
        "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:09}Z",
        system_time.wYear,
        system_time.wMonth,
        system_time.wDay,
        system_time.wHour,
        system_time.wMinute,
        system_time.wSecond,
        nanoseconds);
}

std::string format_iso8601(hires_utc_clock::time_point utc_timestamp, date::time_zone const *time_zone) noexcept
{
    return format_iso8601_utc(utc_timestamp);

    //if (time_zone == nullptr) {
    //    try {
    //        time_zone = date::current_zone();
    //        tt_axiom(time_zone != nullptr);
    //
    //    } catch (std::runtime_error const &) {
    //        return format_iso8601_utc(utc_timestamp);
    //    }
    //}
    //
    //ttlet nanoseconds = utc_timestamp.time_since_epoch().count() % 1000000000;
    //
    //ttlet sys_timestamp = hires_utc_clock::to_system_time_point(utc_timestamp);
    //
    //try {
    //    ttlet local_zoned_time = date::make_zoned(time_zone, sys_timestamp);
    //    ttlet local_time = local_zoned_time.get_local_time();
    //
    //    ttlet daypoint = date::floor<date::days>(local_time);
    //
    //    using CT = typename std::common_type<std::chrono::system_clock::duration, std::chrono::seconds>::type;
    //    ttlet tod = date::time_of_day<CT>{local_time - date::local_seconds{daypoint}};
    //    ttlet seconds = tod.seconds().count();
    //
    //    ttlet tz_offset = local_zoned_time.get_info().offset;
    //
    //    std::string tz_offset_string;
    //    if (tz_offset == 0min) {
    //        tz_offset_string = "Z";
    //    } else if (tz_offset >= 0min) {
    //        ttlet tz_offset_minutes = (tz_offset % 1h).count();
    //        ttlet tz_offset_hours = tz_offset / 1h;
    //        if (tz_offset_minutes == 0) {
    //            tz_offset_string = fmt::format("+{:02}", tz_offset_hours);
    //        } else {
    //            tz_offset_string = fmt::format("+{:02}{:02}", tz_offset_hours, tz_offset_minutes);
    //        }
    //    } else {
    //        ttlet tz_offset_minutes = ((-tz_offset) % 1h).count();
    //        ttlet tz_offset_hours = (-tz_offset) / 1h;
    //        if (tz_offset_minutes == 0) {
    //            tz_offset_string = fmt::format("-{:02}", tz_offset_hours);
    //        } else {
    //            tz_offset_string = fmt::format("-{:02}{:02}", tz_offset_hours, tz_offset_minutes);
    //        }
    //    }
    //
    //    ttlet local_timestring = date::format("%Y-%m-%dT%H:%M", local_zoned_time);
    //
    //    return fmt::format("{}:{:02}.{:09}{}", local_timestring, seconds, nanoseconds, tz_offset_string);
    //
    //} catch (...) {
    //    return format_iso8601_utc(utc_timestamp);
    //}
}

} // namespace tt
