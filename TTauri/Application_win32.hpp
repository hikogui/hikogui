// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application_base.hpp"
#include <Windows.h>
#include <thread>

namespace TTauri {

constexpr UINT WM_APP_LAST_WINDOW_CLOSED = WM_APP + 1;
constexpr UINT WM_APP_OPENING_WINDOW = WM_APP + 2;
constexpr UINT WM_APP_CLOSING_WINDOW = WM_APP + 3;
constexpr UINT WM_APP_CLOSE_WINDOW = WM_APP + 4;
constexpr UINT WM_APP_MINIMIZE_WINDOW = WM_APP + 5;
constexpr UINT WM_APP_MAXIMIZE_WINDOW = WM_APP + 6;
constexpr UINT WM_APP_NORMALIZE_WINDOW = WM_APP + 7;

class Application_win32 final : public Application_base {
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
    void mainThreadLastWindowClosed();

    void startingLoop() override;

    int loop() override;
};

}
