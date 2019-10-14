// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/sync_clock.hpp"
#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/Foundation/audio_counter_clock.hpp"
#include "TTauri/Foundation/cpu_counter_clock.hpp"

namespace TTauri {

FoundationGlobals::FoundationGlobals(std::thread::id main_thread_id, std::string applicationName, URL tzdata_location) noexcept :
    main_thread_id(main_thread_id),
    applicationName(std::move(applicationName))
{
    required_assert(Foundation_globals == nullptr);
    Foundation_globals = this;

    // The logger is the first object that will use the timezone database.
    // Zo we will initialize it here.
#if USE_OS_TZDB == 0
    date::set_install(tzdata_location.nativePath());
#endif
    try {
        time_zone = date::current_zone();
    } catch (std::runtime_error &e) {
        LOG_ERROR("Could not get the current time zone, all times shown as UTC: '{}'", e.what());
    }

    // First we need a clock, it is used by almost any other service.
    // It will imediatly be synchronized, but inaccuratly, it will take a while to become
    // more accurate, but we don't want to block here.
    sync_clock_calibration<hires_utc_clock,cpu_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,cpu_counter_clock>("cpu_utc");

    sync_clock_calibration<hires_utc_clock,audio_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,audio_counter_clock>("audio_utc");

    logger.startLogging();
    logger.startStatisticsLogging();
}

FoundationGlobals::~FoundationGlobals()
{
    // This will log all current counters then all
    // messages that are left in the queue..
    logger.stopStatisticsLogging();
    logger.stopLogging();

    delete sync_clock_calibration<hires_utc_clock,audio_counter_clock>;
    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;

    required_assert(Foundation_globals == this);
    Foundation_globals = nullptr;
}

void FoundationGlobals::maintenanceThreadProcedure()
{
    LOG_INFO("Maintenance thread started.");

    while (!stopMaintenanceThread) {
        std::this_thread::sleep_for(100ms);

        sync_clock_calibration<hires_utc_clock,audio_counter_clock>->calibrate_tick();
        sync_clock_calibration<hires_utc_clock,cpu_counter_clock>->calibrate_tick();

    };

    LOG_INFO("Maintenance thread finished.");
}

void FoundationGlobals::addStaticResource(std::string const &key, gsl::span<std::byte const> value) noexcept
{
    staticResources.try_emplace(key, value);
}

gsl::span<std::byte const> FoundationGlobals::getStaticResource(std::string const &key) const
{
    let i = staticResources.find(key);
    if (i == staticResources.end()) {
        TTAURI_THROW(key_error("Could not find static resource")
            .set<"key"_tag>(key)
        );
    }
    return i->second;
}

}