// Copyright 2019 Pokitec
// All rights reserved.

#include "Application.hpp"
#include "logging.hpp"

namespace TTauri {

using namespace std;


void Application_base::initialize(std::shared_ptr<ApplicationDelegate> applicationDelegate) {
    initializeLogging();
    LOG_INFO("Starting application.");

    this->delegate = std::move(applicationDelegate);
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
