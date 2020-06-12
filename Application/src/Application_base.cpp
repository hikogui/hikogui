// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Application/Application.hpp"
#include "TTauri/Foundation/StaticResourceView.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/globals.hpp"
#include <memory>

namespace tt {

using namespace std;

Application_base::Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate, std::vector<std::string> const &arguments, void *hInstance, int nCmdShow) :
    delegate(applicationDelegate)
{
    ttauri_assert(delegate);

    tt::applicationName = applicationDelegate->applicationName();
    tt::configuration = applicationDelegate->configuration(arguments);
    tt::foundation_startup();

#if defined(BUILD_TTAURI_AUDIO)
    tt::audioDelegate = this;
    tt::audio_startup();
#endif

#if defined(BUILD_TTAURI_GUI)
    tt::text_startup();
#if OPERATING_SYSTEM == OS_WINDOWS
    tt::hInstance = hInstance;
    tt::nCmdShow = nCmdShow;
#endif
    tt::guiDelegate = this;
    tt::gui_startup();
    tt::widgets_startup();
#endif
    LOG_INFO("Starting application '{}'.", applicationName);
}

Application_base::~Application_base()
{
#if defined(BUILD_TTAURI_GUI)
    tt::widgets_shutdown();
    tt::gui_shutdown();
    tt::text_shutdown();
#endif
#if defined(BUILD_TTAURI_AUDIO)
    tt::audio_shutdown();
#endif
    tt::foundation_shutdown();
    LOG_INFO("Stopping application.");
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
