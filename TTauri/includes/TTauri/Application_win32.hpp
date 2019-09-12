// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Application_base.hpp"
#include <Windows.h>
#include <thread>

namespace TTauri {

constexpr UINT WM_APP_CALL_FUNCTION = WM_APP + 1;

class Application_win32 final : public Application_base {
public:
    HINSTANCE hInstance = nullptr;
    HINSTANCE hPrevInstance = nullptr;
    PWSTR pCmdLine = nullptr;
    int nCmdShow = 0;
    DWORD mainThreadID = 0;

    Application_win32(const std::shared_ptr<ApplicationDelegate> delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
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
