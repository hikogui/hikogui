// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Application/Application.hpp"
#include "TTauri/Foundation/StaticResourceView.hpp"
#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Required/globals.hpp"
#include <memory>

namespace TTauri {

using namespace std;


Application_base::Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate, void *hInstance, int nCmdShow) :
    delegate(applicationDelegate),
    i_required(std::this_thread::get_id(), applicationDelegate->applicationName()),
    i_time(URL::urlFromResourceDirectory() / "tzdata"),
    i_diagnostic(),
    i_config(),
    i_draw(),
#if OPERATING_SYSTEM == OS_WINDOWS
    i_gui(this, hInstance, nCmdShow),
#else
    i_gui(this),
#endif
    i_widgets()
{
    required_assert(delegate);
    required_assert(_application == nullptr);
    _application = this;

    LOG_AUDIT("Starting application '{}'.", Required_globals->applicationName);
}

Application_base::~Application_base()
{
    LOG_AUDIT("Stopping application.");

    // Application should be destructed only once.
    required_assert(_application == this);
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
