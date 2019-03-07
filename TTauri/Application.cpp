//
//  Application.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Application.hpp"

namespace TTauri {

std::shared_ptr<Application> app;

Application::Application(const boost::filesystem::path &resourceDir) :
    resourceDir(resourceDir)
{
}

Application::~Application()
{
}

}
