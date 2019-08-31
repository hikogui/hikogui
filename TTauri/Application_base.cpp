// Copyright 2019 Pokitec
// All rights reserved.

#include "Application.hpp"
#include "sync_clock.hpp"
#include "logger.hpp"

namespace TTauri {

using namespace std;


void Application_base::initialize(std::shared_ptr<ApplicationDelegate> applicationDelegate) :
    delegate(std::move(applicationDelegate))
{
    // Application should be instantiated only once.
    required_assert(Application_base::singleton == nullptr);
    Application_base::singleton = this;

    LOG_AUDIT("Starting application.");

    // First we need a clock, it is used by almost any other service.
    // It will imediatly be synchronized, but inaccuratly, it will take a while to become
    // more accurate, but we don't want to block here.
    sync_clock_calibration<hires_utc_clock,cpu_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,cpu_counter_clock>();

    // Next we need the logger thread, the logger can already buffer
    // a certain number of messages, but this buffer need to be services or the
    // log functions will eventually block.
    logger.startLogging();
}

Application_base::~Application_base()
{
    // Application should be destructed only once.
    required_assert(Application_base::singleton == this);

    LOG_AUDIT("Stopping application.");

    // Stop logger before clock is removed.
    // This will log all current counters then all
    // messages that are left in the queue..
    logger.stopLogging();

    // The clock should already have a calibration.
    required_assert(sync_clock_calibration<hires_utc_clock,cpu_counter_clock> != nullptr);
    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;

    Application_base::singleton = nullptr;
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
