//
//  Application.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Application.hpp"

#include "Logging.hpp"

namespace TTauri {

using namespace std;

std::shared_ptr<Application> Application::singleton;

Application::Application(std::shared_ptr<Delegate> delegate) :
    delegate(move(delegate))
{
    initializeLogging();
    LOG_INFO("Starting application.");
}

void Application::startingLoop()
{
    if (!loopStarted) {
        loopStarted = true;
        delegate->startingLoop();
    }
}

void Application::lastWindowClosed()
{
    delegate->lastWindowClosed();
}

}
