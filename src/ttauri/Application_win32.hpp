// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application_base.hpp"
#include <thread>

namespace tt {

class Application_win32 final : public Application_base {
public:
    uint32_t mainThreadID = 0;

    Application_win32(std::shared_ptr<ApplicationDelegate> delegate, void *hInstance, int nCmdShow);
    ~Application_win32() = default;

    Application_win32(const Application_win32 &) = delete;
    Application_win32 &operator=(const Application_win32 &) = delete;
    Application_win32(Application_win32 &&) = delete;
    Application_win32 &operator=(Application_win32 &&) = delete;

    void lastWindowClosed() override;

    void runOnMainThread(std::function<void()> function) override;

    int loop() override;

protected:
    bool startingLoop() override;

    void audioStart() override;

};

}
