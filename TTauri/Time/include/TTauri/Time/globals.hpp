// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Time/sync_clock.hpp"
#include "TTauri/Time/cpu_counter_clock.hpp"
#include "TTauri/Time/hires_utc_clock.hpp"
#include "TTauri/Required/required.hpp"
#include <fmt/format.h>
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace TTauri {

struct time_globals_type {
    date::time_zone const *time_zone = nullptr;
    std::optional<std::string> time_zone_error_message;

    time_globals_type(std::string tzdata_path) {
        // The logger is the first object that will use the timezone database.
        // Zo we will initialize it here.
#if USE_OS_TZDB == 0
        date::set_install(tzdata_path);
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
            new sync_clock_calibration_type<hires_utc_clock,cpu_counter_clock>("hiperf_utc");
    }

    ~time_globals_type() {
        delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;
    }

    std::optional<std::string> read_message(void) {
        if (time_zone_error_message) {
            let message = time_zone_error_message;
            time_zone_error_message = {};
            return message;
        } else {
            return sync_clock_calibration<hires_utc_clock,cpu_counter_clock>->read_message();
        }
    }
};

inline std::unique_ptr<time_globals_type> time_globals;

}
