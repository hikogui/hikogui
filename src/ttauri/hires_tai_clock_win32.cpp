// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_tai_clock.hpp"
#include "subsystem.hpp"
#include "logger.hpp"
#include <Windows.h>

namespace tt {

bool hires_tai_clock::subsystem_init() noexcept
{
    PROCESS_LEAP_SECOND_INFO LeapSecondInfo;
    ZeroMemory(&LeapSecondInfo, sizeof(LeapSecondInfo));
    LeapSecondInfo.Flags = PROCESS_LEAP_SECOND_INFO_FLAG_ENABLE_SIXTY_SECOND;

    auto Success = SetProcessInformation(GetCurrentProcess(), ProcessLeapSecondInfo, &LeapSecondInfo, sizeof(LeapSecondInfo));
    if (!Success) {
        tt_log_fatal("Set Leap Second priority failed: {}\n", tt::get_last_error_message());
    }

    return true;
}

void hires_tai_clock::subsystem_deinit() noexcept {}

hires_tai_clock::time_point hires_tai_clock::now() noexcept
{
    hires_tai_clock::subsystem_start();

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

} // namespace tt
