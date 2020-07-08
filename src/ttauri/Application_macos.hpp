// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application_base.hpp"

namespace tt {

class Application_macos final : public Application_base {
public:
    int argc;
    char const * const *argv;

    Application_macos(const std::shared_ptr<ApplicationDelegate> delegate, int argc, char const * const *argv);
    ~Application_macos();

    void runOnMainThread(std::function<void()> function) override;

    void lastWindowClosed() override {}

    int loop() override;
};

}
