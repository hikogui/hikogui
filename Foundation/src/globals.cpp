// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/sync_clock.hpp"
#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/Foundation/audio_counter_clock.hpp"
#include "TTauri/Foundation/cpu_counter_clock.hpp"
#include "TTauri/Foundation/trace.hpp"

#include "data/UnicodeData.bin.inl"

namespace TTauri {

FoundationGlobals::FoundationGlobals(std::thread::id main_thread_id, datum configuration, std::string applicationName, URL tzdata_location) noexcept :
    main_thread_id(main_thread_id),
    configuration(std::move(configuration)),
    applicationName(std::move(applicationName))
{
    required_assert(Foundation_globals == nullptr);
    Foundation_globals = this;

    // The logger is the first object that will use the timezone database.
    // So we will initialize it here.
#if USE_OS_TZDB == 0
    date::set_install(tzdata_location.nativePath());
#endif
    try {
        time_zone = date::current_zone();
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


    Foundation_globals->addStaticResource(UnicodeData_bin_filename, UnicodeData_bin_bytes);
    unicodeData = parseResource<UnicodeData>(URL("resource:UnicodeData.bin"));

    maintenanceThread = std::thread([=]() {
        return this->maintenanceThreadProcedure();
    });
}

FoundationGlobals::~FoundationGlobals()
{
    stopMaintenanceThread();

    delete sync_clock_calibration<hires_utc_clock,audio_counter_clock>;
    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;

    required_assert(Foundation_globals == this);
    Foundation_globals = nullptr;
}

void FoundationGlobals::stopMaintenanceThread() noexcept
{
    auto lock = std::scoped_lock(mutex);

    _stopMaintenanceThread = true;
    if (maintenanceThread.joinable()) {
        maintenanceThread.join();
    }
}

void FoundationGlobals::maintenanceThreadProcedure() noexcept
{
    set_thread_name("FoundationMaintenance");
    LOG_INFO("Maintenance thread started.");

    while (!_stopMaintenanceThread) {
        std::this_thread::sleep_for(100ms);

        let t = trace<"maintenance"_tag>{};

        {
            let t = trace<"calibrate_clk"_tag>{};
            sync_clock_calibration<hires_utc_clock,audio_counter_clock>->calibrate_tick();
            sync_clock_calibration<hires_utc_clock,cpu_counter_clock>->calibrate_tick();
        }

        logger.gather_tick(false);
        logger.logger_tick();
    };
    LOG_INFO("Maintenance thread finishing.");

    // Before the maintenance thread is terminated, gather all statistics and
    // make sure all messages are logged.
    logger.gather_tick(true);
    logger.logger_tick();
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