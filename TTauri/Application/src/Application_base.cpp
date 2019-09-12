// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Application.hpp"
#include "TTauri/StaticResourceView.hpp""
#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Required/globals.hpp"
#include "TTauri/GUI/init.hpp"

namespace TTauri {

using namespace std;


Application_base::Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate) :
    delegate(std::move(applicationDelegate))
{
    required_assert(delegate);

    // Application should be instantiated only once.
    required_assert(_application == nullptr);
    _application = this;

    applicationName = delegate->applicationName();

    LOG_AUDIT("Starting application '{}'.", applicationName);

    let tzdata_location = URL::urlFromResourceDirectory() / "tzdata";
    time_globals = std::make_unique<time_globals_type>(tzdata_location.nativePath());

    GUI::GUI_init();

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

    time_globals = {};
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
