// Copyright 2019 Pokitec
// All rights reserved.

#include "globals.hpp"
#include "sync_clock.hpp"
#include "hires_utc_clock.hpp"
#include "audio_counter_clock.hpp"
#include "cpu_counter_clock.hpp"
#include "trace.hpp"
#include "timer.hpp"

namespace tt {

/** Reference counter to determine the amount of startup/shutdowns.
*/
static std::atomic<uint64_t> startupCount = 0;

std::unordered_map<std::string,nonstd::span<std::byte const>> staticResources;

size_t logger_maintenance_cbid;
size_t clock_maintenance_cbid;

void addStaticResource(std::string const &key, nonstd::span<std::byte const> value) noexcept
{
    staticResources.try_emplace(key, value);
}

nonstd::span<std::byte const> getStaticResource(std::string const &key)
{
    ttlet i = staticResources.find(key);
    if (i == staticResources.end()) {
        TTAURI_THROW(key_error("Could not find static resource")
            .set<key_tag>(key)
        );
    }
    return i->second;
}

void foundation_startup()
{
    if (startupCount.fetch_add(1) != 0) {
        // The library has already been initialized.
        return;
    }
    LOG_INFO("TTauri startup");

    mainThreadID = std::this_thread::get_id();

    logger.minimum_log_level = static_cast<log_level>(static_cast<int>(configuration["log-level"]));

    // The logger is the first object that will use the timezone database.
    // So we will initialize it here.
#if USE_OS_TZDB == 0
    ttlet tzdata_location = URL::urlFromResourceDirectory() / "tzdata";
    date::set_install(tzdata_location.nativePath());
#endif
    try {
        timeZone = date::current_zone();
    } catch (std::runtime_error &e) {
        LOG_ERROR("Could not get the current time zone, all times shown as UTC: '{}'", e.what());
    }

    // First we need a clock, it is used by almost any other service.
    // It will immediately be synchronized, but inaccurately, it will take a while to become
    // more accurate, but we don't want to block here.
    sync_clock_calibration<hires_utc_clock,cpu_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,cpu_counter_clock>("cpu_utc");

    sync_clock_calibration<hires_utc_clock,audio_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,audio_counter_clock>("audio_utc");

    logger_maintenance_cbid = maintenance_timer.add_callback(100ms, [](auto current_time, auto last) {
        struct logger_maintenance_tag {};
        ttlet t2 = trace<logger_maintenance_tag>{};

        logger.gather_tick(last);
        logger.logger_tick();
    });

    clock_maintenance_cbid = maintenance_timer.add_callback(100ms, [](auto...) {
        struct clock_maintenance_tag {};
        ttlet t2 = trace<clock_maintenance_tag>{};

        sync_clock_calibration<hires_utc_clock,audio_counter_clock>->calibrate_tick();
        sync_clock_calibration<hires_utc_clock,cpu_counter_clock>->calibrate_tick();
    });
}

void foundation_shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_INFO("TTauri shutdown");

    // Force all timers to finish.
    maintenance_timer.stop();
    maintenance_timer.remove_callback(clock_maintenance_cbid);
    maintenance_timer.remove_callback(logger_maintenance_cbid);

    delete sync_clock_calibration<hires_utc_clock,audio_counter_clock>;
    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;
}
    
}
