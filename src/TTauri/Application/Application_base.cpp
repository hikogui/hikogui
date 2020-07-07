// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Application/Application.hpp"
#include "TTauri/Foundation/StaticResourceView.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/globals.hpp"
#include <memory>

namespace tt {

using namespace std;

Application_base::Application_base(
    std::shared_ptr<ApplicationDelegate> applicationDelegate,
    std::vector<std::string> const &arguments,
    void *_hInstance,
    int _nCmdShow
) :
    delegate(applicationDelegate)
{
    tt_assert(delegate);

    tt::applicationName = applicationDelegate->applicationName();
    tt::configuration = applicationDelegate->configuration(arguments);
    tt::foundation_startup();

    tt::audioDelegate = this;
    tt::audio_startup();

    tt::text_startup();
#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    tt::hInstance = _hInstance;
    tt::nCmdShow = _nCmdShow;
#endif
    tt::guiDelegate = this;
    tt::gui_startup();
    tt::widgets_startup();

    LOG_INFO("Starting application '{}'.", applicationName);
}

Application_base::~Application_base()
{
    tt::widgets_shutdown();
    tt::gui_shutdown();
    tt::text_shutdown();

    tt::audio_shutdown();

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

void Application_base::audioDeviceListChanged()
{
    delegate->audioDeviceListChanged();
}

}
