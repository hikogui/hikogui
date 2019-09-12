// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application_base.hpp"

namespace TTauri {

class Application_macos final : public Application_base {
public:
    int argc;
    const char **argv;

    Application_macos();
    ~Application_macos();

    void initialize(const std::shared_ptr<ApplicationDelegate> delegate, int argc, char **argv);

    void runOnMainThread(std::function<void()> function) override;

    int loop() override;
};

}
