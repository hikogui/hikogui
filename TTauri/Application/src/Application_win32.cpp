// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Application/Application_win32.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/Required/strings.hpp"
#include <vulkan/vulkan.hpp>
#include <thread>
#include <string>

namespace TTauri {


Application_win32::Application_win32(const std::shared_ptr<ApplicationDelegate> delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) :
    Application_base(std::move(delegate), hInstance, nCmdShow),
    hInstance(hInstance), hPrevInstance(hPrevInstance), pCmdLine(pCmdLine), nCmdShow(nCmdShow),
    mainThreadID(GetCurrentThreadId())
{
}

void Application_win32::lastWindowClosed()
{
    runOnMainThread([&]() {
        // Let the application have a change to open new windows from the main thread.
        Application_base::lastWindowClosed();

        if (GUI::GUI_globals->instance().getNumberOfWindows() == 0) {
            LOG_INFO("Application quiting due to all windows having been closed.");
            PostQuitMessage(0);
        }
    });
}

gsl_suppress(r.11)
void Application_win32::runOnMainThread(std::function<void()> function)
{
    let functionP = new std::function<void()>(std::move(function));
    required_assert(functionP);

    auto r = PostThreadMessageW(mainThreadID, WM_APP_CALL_FUNCTION, 0, reinterpret_cast<LPARAM>(functionP));
    required_assert(r != 0);
}

void Application_win32::startingLoop()
{
    Application_base::startingLoop();
}

int Application_win32::loop()
{
    startingLoop();


    Required_globals->main_thread_runner = [=](std::function<void()> f) {
        return this->runOnMainThread(f);
    };

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        switch (msg.message) {
        case WM_APP_CALL_FUNCTION: {
            let functionP = reinterpret_cast<std::function<void()> *>(msg.lParam);
            (*functionP)();
            delete functionP;
            } break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Required_globals->main_thread_runner = {};

    return 0;
}

}
