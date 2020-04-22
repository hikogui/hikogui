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
    delegate(applicationDelegate)
{
    ttauri_assert(delegate);

    TTauri::applicationName = applicationDelegate->applicationName();
    TTauri::configuration = applicationDelegate->configuration(arguments);
    TTauri::startup();

#if defined(BUILD_TTAURI_AUDIO)
    TTauri::Audio::audioDelegate = this;
    TTauri::Audio::startup();
#endif

#if defined(BUILD_TTAURI_GUI)
    TTauri::Text::startup();
#if OPERATING_SYSTEM == OS_WINDOWS
    TTauri::GUI::hInstance = hInstance;
    TTauri::GUI::nCmdShow = nCmdShow;
#endif
    TTauri::GUI::guiDelegate = this;
    TTauri::GUI::startup();
    TTauri::GUI::Widgets::startup();
#endif
    LOG_INFO("Starting application '{}'.", applicationName);
}

Application_base::~Application_base()
{
#if defined(BUILD_TTAURI_GUI)
    TTauri::GUI::Widgets::shutdown();
    TTauri::GUI::shutdown();
    TTauri::Text::shutdown();
#endif
#if defined(BUILD_TTAURI_AUDIO)
    TTauri::Audio::shutdown();
#endif
    TTauri::shutdown();
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
