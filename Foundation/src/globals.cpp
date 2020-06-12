// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/sync_clock.hpp"
#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/Foundation/audio_counter_clock.hpp"
#include "TTauri/Foundation/cpu_counter_clock.hpp"
#include "TTauri/Foundation/trace.hpp"
#include "TTauri/Foundation/config.hpp"

namespace tt {

/** Reference counter to determine the amount of startup/shutdowns.
*/
static std::atomic<uint64_t> startupCount = 0;


std::unordered_map<std::string,nonstd::span<std::byte const>> staticResources;

/** The maintenance thread.
*/
std::thread maintenanceThread;

std::recursive_mutex mutex;

/** A flag to stop the maintenance thread.
*/
bool _stopMaintenanceThread = false;

void stopMaintenanceThread() noexcept
{
    auto lock = std::scoped_lock(mutex);

    _stopMaintenanceThread = true;
    if (maintenanceThread.joinable()) {
        maintenanceThread.join();
    }
}

static void maintenanceThreadProcedure() noexcept
{
    set_thread_name("FoundationMaintenance");
    LOG_INFO("Maintenance thread started.");

    while (!_stopMaintenanceThread) {
        std::this_thread::sleep_for(100ms);

        struct maintenance_tag {};
        ttlet t1 = trace<maintenance_tag>{};

        {
            struct calibrate_tag {};
            ttlet t2 = trace<calibrate_tag>{};
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

    maintenanceThread = std::thread(maintenanceThreadProcedure);
}

void foundation_shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_INFO("TTauri shutdown");

    stopMaintenanceThread();

    delete sync_clock_calibration<hires_utc_clock,audio_counter_clock>;
    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;
}
    
}
