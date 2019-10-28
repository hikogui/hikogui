// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Application/Application.hpp"
#include "TTauri/Foundation/StaticResourceView.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/globals.hpp"
#include <memory>

namespace TTauri {

using namespace std;


Application_base::Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate, void *hInstance, int nCmdShow) :
    delegate(applicationDelegate),
    i_foundation(std::this_thread::get_id(), applicationDelegate->applicationName(), URL::urlFromResourceDirectory() / "tzdata"),
#if defined(TTAURI_CONFIG_ENABLED)
    i_config(),
#endif
#if defined(TTAURI_AUDIO_ENABLED)
    i_audio(this),
#endif
#if defined(TTAURI_GUI_ENABLED)
#if OPERATING_SYSTEM == OS_WINDOWS
    i_gui(this, hInstance, nCmdShow),
#else
    i_gui(this),
#endif
    i_widgets(),
#endif
    i_dummy()
{
    required_assert(delegate);
    required_assert(_application == nullptr);
    _application = this;

    LOG_AUDIT("Starting application '{}'.", Foundation_globals->applicationName);
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

#if defined(TTAURI_GUI_ENABLED)
void Application_base::lastWindowClosed()
{
    delegate->lastWindowClosed();
}
#endif

#if defined(TTAURI_AUDIO_ENABLED)
void Application_base::audioDeviceListChanged()
{
    delegate->audioDeviceListChanged();
}
#endif

}
