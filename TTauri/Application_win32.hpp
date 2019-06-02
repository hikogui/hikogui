// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application_base.hpp"
#include <Windows.h>
#include <thread>

namespace TTauri {

    const UINT WM_APP_LAST_WINDOW_CLOSED = WM_APP + 1;
    const UINT WM_APP_OPENING_WINDOW = WM_APP + 2;
    const UINT WM_APP_CLOSING_WINDOW = WM_APP + 3;

class Application_win32 : public Application_base {
public:
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    PWSTR pCmdLine;
    int nCmdShow;
    DWORD mainThreadID;

    Application_win32(const std::shared_ptr<ApplicationDelegate> delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
    ~Application_win32() {}

    Application_win32(const Application_win32 &) = delete;
    Application_win32 &operator=(const Application_win32 &) = delete;
    Application_win32(Application_win32 &&) = delete;
    Application_win32 &operator=(Application_win32 &&) = delete;

    void lastWindowClosed() override;
    void mainThreadLastWindowClose();

    int loop() override;
};
}
