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

Application::Application(std::shared_ptr<Delegate> delegate, std::vector<const char *> vulkanExtensions) :
    delegate(delegate)
{
    initializeLogging();
    LOG_INFO("Starting application.");

    instance = std::make_shared<GUI::Instance>(vulkanExtensions);
    instance->setPreferedDeviceUUID({});
}

Application::~Application()
{
}

void Application::initialize()
{
    delegate->initialize();    
}

std::shared_ptr<Application> Application::shared;
}
