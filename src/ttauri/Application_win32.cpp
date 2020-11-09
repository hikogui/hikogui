// Copyright 2019 Pokitec
// All rights reserved.

#include "Application_win32.hpp"
#include "audio/AudioSystem_win32.hpp"
#include "GUI/GUISystem.hpp"
#include "GUI/Window.hpp"
#include "strings.hpp"
#include "timer.hpp"
#include <thread>
#include <string>
#include <vector>
#include <Windows.h>

namespace tt {

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

Application_win32::Application_win32(ApplicationDelegate &delegate, void *hInstance, int nCmdShow) :
    Application_base(delegate, passArguments()), OSMainThreadID(GetCurrentThreadId()), hInstance(hInstance), nCmdShow(nCmdShow)
{
}

void Application_win32::lastWindowClosed()
{
    runFromMainLoop([&]() {
        // Let the application have a chance to open new windows from the main thread.
        delegate.lastWindowClosed();

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

static BOOL CALLBACK win32_windows_EnumThreadWndProc(_In_ HWND hwnd, _In_ LPARAM lParam) noexcept
{
    auto v = std::launder(reinterpret_cast<std::vector<void *> *>(lParam));
    v->push_back(reinterpret_cast<void *>(hwnd));
    return true;
}

[[nodiscard]] std::vector<void *> Application_win32::win32_windows() noexcept
{
    std::vector<void *> windows;
    EnumThreadWindows(OSMainThreadID, win32_windows_EnumThreadWndProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

void Application_win32::post_message(void *window, unsigned int Msg, ptrdiff_t wParam, ptrdiff_t lParam) noexcept
{
    ttlet ret = PostMessageW(
        reinterpret_cast<HWND>(window), static_cast<UINT>(Msg), static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));

    if (!ret) {
        LOG_FATAL("Could not post message {} to window {}: {}", Msg, reinterpret_cast<ptrdiff_t>(window), getLastErrorMessage());
    }
}

void Application_win32::post_message(
    std::vector<void *> const &windows,
    unsigned int Msg,
    ptrdiff_t wParam,
    ptrdiff_t lParam) noexcept
{
    for (auto window : windows) {
        post_message(window, Msg, wParam, lParam);
    }
}

bool Application_win32::initializeApplication()
{
    languages_maintenance_callback = maintenance_timer.add_callback(1s, [this](auto...) {
        ttlet current_language_tags = language::get_preferred_language_tags();
        static auto previous_language_tags = current_language_tags;
        
        if (previous_language_tags != current_language_tags) {
            previous_language_tags = current_language_tags;
            this->post_message(this->win32_windows(), WM_WIN_LANGUAGE_CHANGE);
        }
    });

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

} // namespace tt
