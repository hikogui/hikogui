// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application_base.hpp"

namespace tt {

class Application_macos final : public application {
public:
    Application_macos(const std::shared_ptr<application_delegate> delegate);
    ~Application_macos();

    void runFromMainLoop(std::function<void()> function) override;

    void lastWindowClosed() override {}

    int loop() override;
};

}
