// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Application/Application.hpp"
#include "TTauri/Foundation/StaticResourceView.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/globals.hpp"
#include <memory>

namespace TTauri {

using namespace std;

Application_base::Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate, std::vector<std::string> const &arguments, void *hInstance, int nCmdShow) :
    delegate(applicationDelegate),
    i_foundation(std::this_thread::get_id(), applicationDelegate->configuration(arguments), applicationDelegate->applicationName(), URL::urlFromResourceDirectory() / "tzdata"),
#if defined(BUILD_TTAURI_AUDIO)
    i_audio(this),
#endif
#if defined(BUILD_TTAURI_GUI)
    i_text(),
#if OPERATING_SYSTEM == OS_WINDOWS
    i_gui(this, hInstance, nCmdShow),
#else
    i_gui(this),
#endif
    i_widgets(),
#endif
    i_dummy()
{
    ttauri_assert(delegate);
    ttauri_assert(_application == nullptr);
    _application = this;

    LOG_AUDIT("Starting application '{}'.", Foundation_globals->applicationName);
}

Application_base::~Application_base()
{
    LOG_AUDIT("Stopping application.");

    // Application should be destructed only once.
    ttauri_assert(_application == this);
    _application = nullptr;
}

bool Application_base::startingLoop()
{
    try {
        return delegate->startingLoop();
    } catch (error &e) {
        LOG_FATAL("Exception during startingLoop {}", to_string(e));
    }
}

#if defined(BUILD_TTAURI_AUDIO)
void Application_base::audioDeviceListChanged()
{
    delegate->audioDeviceListChanged();
}
#endif

}
