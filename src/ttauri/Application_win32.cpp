// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/Application_win32.hpp"
#include "TTauri/GUI/GUISystem.hpp"
#include "TTauri/GUI/Window.hpp"
#include "ttauri/strings.hpp"
#include <thread>
#include <string>
#include <vector>
#include <Windows.h>

namespace tt {

constexpr UINT WM_APP_CALL_FUNCTION = WM_APP + 1;

[[nodiscard]] static std::vector<std::string> passArguments() noexcept
{
    std::vector<std::string> arguments;

    ttlet commandLine = GetCommandLineW();

    int argc = 0;
    ttlet argv = CommandLineToArgvW(commandLine, &argc);
    tt_assert(argv != nullptr);

    for (int i = 0; i < argc; i++) {
        arguments.push_back(to_string(std::wstring(argv[i])));
    }

    LocalFree(argv);
    return arguments;
}

Application_win32::Application_win32(std::shared_ptr<ApplicationDelegate> delegate, void *_hInstance, int _nCmdShow) :
    Application_base(std::move(delegate), passArguments(), _hInstance, _nCmdShow),
    mainThreadID(GetCurrentThreadId())
{
}

void Application_win32::lastWindowClosed()
{
    runOnMainThread([&]() {
        // Let the application have a chance to open new windows from the main thread.
        delegate->lastWindowClosed();

        if (guiSystem->getNumberOfWindows() == 0) {
            LOG_INFO("Application quiting due to all windows having been closed.");
            PostQuitMessage(0);
        }
    });
}

gsl_suppress(r.11)
void Application_win32::runOnMainThread(std::function<void()> function)
{
    ttlet functionP = new std::function<void()>(std::move(function));
    tt_assert(functionP);

    auto r = PostThreadMessageW(mainThreadID, WM_APP_CALL_FUNCTION, 0, reinterpret_cast<LPARAM>(functionP));
    tt_assert(r != 0);
}

bool Application_win32::startingLoop()
{
    return Application_base::startingLoop();
}

int Application_win32::loop()
{
    if (!startingLoop()) {
        return 0;
    }

    mainThreadRunner = [=](std::function<void()> f) {
        return this->runOnMainThread(f);
    };

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        switch (msg.message) {
        case WM_APP_CALL_FUNCTION: {
            ttlet functionP = reinterpret_cast<std::function<void()> *>(msg.lParam);
            (*functionP)();
            delete functionP;
            } break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    mainThreadRunner = {};
    return 0;
}

}
