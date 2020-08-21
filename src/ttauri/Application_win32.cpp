// Copyright 2019 Pokitec
// All rights reserved.

#include "Application_win32.hpp"
#include "audio/AudioSystem_win32.hpp"
#include "GUI/GUISystem.hpp"
#include "GUI/Window.hpp"
#include "strings.hpp"
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

Application_win32::Application_win32(std::shared_ptr<ApplicationDelegate> delegate, void *hInstance, int nCmdShow) :
    Application_base(std::move(delegate), passArguments()),
    OSMainThreadID(GetCurrentThreadId()),
    hInstance(hInstance), nCmdShow(nCmdShow)
{
}

void Application_win32::lastWindowClosed()
{
    runFromMainLoop([&]() {
        // Let the application have a chance to open new windows from the main thread.
        delegate->lastWindowClosed();

        if (gui->getNumberOfWindows() == 0) {
            LOG_INFO("Application quiting due to all windows having been closed.");
            PostQuitMessage(0);
        }
    });
}

void Application_win32::runFromMainLoop(std::function<void()> function)
{
    tt_assert(inLoop);

    ttlet functionP = new std::function<void()>(std::move(function));
    tt_assert(functionP);

    auto r = PostThreadMessageW(OSMainThreadID, WM_APP_CALL_FUNCTION, 0, reinterpret_cast<LPARAM>(functionP));
    tt_assert(r != 0);
}

bool Application_win32::initializeApplication()
{
    return Application_base::initializeApplication();
}

int Application_win32::loop()
{
    inLoop = true;

    if (!initializeApplication()) {
        return 0;
    }

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

    inLoop = false;
    return 0;
}

void Application_win32::audioStart()
{
    Application_base::audioStart();
    audio = std::make_unique<AudioSystem_win32>(this);
}

}
