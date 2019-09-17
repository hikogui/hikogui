// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/globals.hpp"

namespace TTauri {

TimeGlobals::TimeGlobals(URL tzdata_location)
{
    required_assert(Required_globals != nullptr);
    required_assert(Time_globals == nullptr);
    Time_globals = this;

    // The logger is the first object that will use the timezone database.
    // Zo we will initialize it here.
#if USE_OS_TZDB == 0
    date::set_install(tzdata_location.nativePath());
#endif
    try {
        time_zone = date::current_zone();
    } catch (std::runtime_error &e) {
        time_zone_error_message = fmt::format("Could not get the current time zone, all times shown as UTC: '{}'", e.what());
    }

    // First we need a clock, it is used by almost any other service.
    // It will imediatly be synchronized, but inaccuratly, it will take a while to become
    // more accurate, but we don't want to block here.
    sync_clock_calibration<hires_utc_clock,cpu_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,cpu_counter_clock>("cpu_utc");

    sync_clock_calibration<hires_utc_clock,audio_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,audio_counter_clock>("audio_utc");
}

TimeGlobals::~TimeGlobals()
{
    delete sync_clock_calibration<hires_utc_clock,audio_counter_clock>;
    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;

    required_assert(Time_globals == this);
    Time_globals = nullptr;
}

std::optional<std::string> TimeGlobals::read_message(void) noexcept
{
    if (time_zone_error_message) {
        let message = time_zone_error_message;
        time_zone_error_message = {};
        return message;

    } else if (let cpu_utc_message = sync_clock_calibration<hires_utc_clock,cpu_counter_clock>->read_message()) {
        return cpu_utc_message;

    } else if (let audio_utc_message = sync_clock_calibration<hires_utc_clock,audio_counter_clock>->read_message()) {
        return audio_utc_message;

    } else {
        return {};
    }
}

}