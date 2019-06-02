// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application.hpp"

namespace TTauri {

class Application_macos: public Application {
public:
    int argc;
    const char **argv;

    Application_macos(const std::shared_ptr<ApplicationDelegate> &delegate, int argc, const char **arg);
    ~Application_macos();

    virtual int loop();
};

}
