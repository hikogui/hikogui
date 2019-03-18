
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
    ~Application_win32();

    virtual int loop();
};
}
