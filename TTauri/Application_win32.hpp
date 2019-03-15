
#pragma once

#include "Application.hpp"

#include <Windows.h>

#include <thread>

namespace TTauri {

class Application_win32 : public Application {
public:
    HINSTANCE win32Instance;
    int win32Show;

    std::shared_ptr<std::thread> redrawThread;

    Application_win32(std::shared_ptr<Delegate> delegate, HINSTANCE instance, PWSTR commandLine, int show);
    ~Application_win32();

    virtual std::shared<Window> createWindow(std::shared_ptr<Window::Delegate> windowDelegate, const std::string &title);
    virtual int loop();
};

}
