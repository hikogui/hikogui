
#pragma once

#include "Application.hpp"

#include <Windows.h>

#include <thread>

namespace TTauri {

class Application_win32 : public Application {
public:
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    PWSTR pCmdLine;
    int nCmdShow;

    Application_win32(const std::shared_ptr<Delegate> &delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
    ~Application_win32() {}

    Application_win32(const Application_win32 &) = delete;
    Application_win32 &operator=(const Application_win32 &) = delete;
    Application_win32(Application_win32 &&) = delete;
    Application_win32 &operator=(Application_win32 &&) = delete;

    int loop() override;
};
}
