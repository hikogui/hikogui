// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Application/Application_base.hpp"
#include <thread>

namespace TTauri {

class Application_win32 final : public Application_base {
public:
    void *hInstance = nullptr;
    void *hPrevInstance = nullptr;
    wchar_t const *pCmdLine = nullptr;
    int nCmdShow = 0;
    uint32_t mainThreadID = 0;

    Application_win32(const std::shared_ptr<ApplicationDelegate> delegate, void *hInstance, void *hPrevInstance, wchar_t const *pCmdLine, int nCmdShow);
    ~Application_win32() = default;

    Application_win32(const Application_win32 &) = delete;
    Application_win32 &operator=(const Application_win32 &) = delete;
    Application_win32(Application_win32 &&) = delete;
    Application_win32 &operator=(Application_win32 &&) = delete;

    void lastWindowClosed() override;

    void runOnMainThread(std::function<void()> function) override;

    void startingLoop() override;

    int loop() override;
};

}
