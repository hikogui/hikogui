// Copyright 2019 Pokitec
// All rights reserved.

#include "Application.hpp"
#include "hiperf_utc_clock.hpp"
#include "logger.hpp"

namespace TTauri {

using namespace std;


Application_base::Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate) :
    delegate(std::move(applicationDelegate))
{
    // Application should be instantiated only once.
    required_assert(_application == nullptr);
    _application = this;

    LOG_AUDIT("Starting application.");

    // The logger is the first object that will use the timezone database.
    // Zo we will initialize it here.
#if USE_OS_TZDB == 0
    let tzdata_location = URL::urlFromResourceDirectory() / "tzdata";
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
        new sync_clock_calibration_type<hires_utc_clock,cpu_counter_clock>("hiperf_utc");

    // Next we need the logger thread, the logger can already buffer
    // a certain number of messages, but this buffer need to be services or the
    // log functions will eventually block.
    logger.startLogging();
    logger.startStatisticsLogging();
}

Application_base::~Application_base()
{
    // Application should be destructed only once.
    required_assert(_application == this);

    LOG_AUDIT("Stopping application.");

    // Stop logger before clock is removed.
    // This will log all current counters then all
    // messages that are left in the queue..
    logger.stopStatisticsLogging();
    logger.stopLogging();

    // The clock should already have a calibration.
    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;

    _application = nullptr;
}

void Application_base::startingLoop()
{
    if (!loopStarted) {
        loopStarted = true;
        delegate->startingLoop();
    }
}

void Application_base::lastWindowClosed()
{
    delegate->lastWindowClosed();
}

}
