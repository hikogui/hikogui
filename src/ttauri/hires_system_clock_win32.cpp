// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_system_clock.hpp"
#include "hires_tai_clock.hpp"
#include <Windows.h>
#include <date/date.h>

namespace tt {

hires_system_clock::time_point hires_system_clock::now() noexcept
{
    using namespace std::chrono_literals;

    hires_tai_clock::subsystem_start();

    // Get the file_time, which since 2018 tracks leap-seconds.
    FILETIME file_time;
    GetSystemTimePreciseAsFileTime(&file_time);

    // The system time will contain UTC separated into date and time fields.
    // the 'wSecond' field may indicate 60 during a leap second.
    SYSTEMTIME system_time;
    ttlet r = FileTimeToSystemTime(&file_time, &system_time);
    tt_assert(r);

    // Get the number of days since 1970-01-01.
    ttlet date = date::year_month_day{date::year{system_time.wYear}, date::month{system_time.wMonth}, date::day{system_time.wDay}};
    int64_t ns1970 = static_cast<std::chrono::sys_days>(date).time_since_epoch() / 1h;
    ns1970 += system_time.wHour;
    ns1970 *= 60;
    ns1970 += system_time.wMinute;
    ns1970 *= 60;
    ns1970 += std::min(system_time.wSecond, narrow_cast<WORD>(59)); // Drop leap-second for UTC.

    auto time_in_100ns = static_cast<int64_t>(file_time.dwHighDateTime) << 32;
    time_in_100ns |= static_cast<int64_t>(file_time.dwLowDateTime);
    auto time_in_ns = time_in_100ns * 100;

    ns1970 *= 1'000'000'000;
    ns1970 += time_in_ns % static_cast<int64_t>(1'000'000'000);

    return time_point(duration(ns1970));
}

} // namespace tt
