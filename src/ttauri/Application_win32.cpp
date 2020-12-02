// Copyright 2019 Pokitec
// All rights reserved.

#include "application_win32.hpp"
#include "audio/audio_system_aggregate.hpp"
#include "audio/audio_system_win32.hpp"
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

application_win32::application_win32(std::weak_ptr<application_delegate> const &delegate, void *hInstance, int nCmdShow) :
    application(delegate, passArguments()), OSMainThreadID(GetCurrentThreadId()), hInstance(hInstance), nCmdShow(nCmdShow)
{
}

void application_win32::run_from_main_loop(std::function<void()> function)
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

[[nodiscard]] std::vector<void *> application_win32::win32_windows() noexcept
{
    std::vector<void *> windows;
    EnumThreadWindows(OSMainThreadID, win32_windows_EnumThreadWndProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

void application_win32::post_message(void *window, unsigned int Msg, ptrdiff_t wParam, ptrdiff_t lParam) noexcept
{
    ttlet ret = PostMessageW(
        reinterpret_cast<HWND>(window), static_cast<UINT>(Msg), static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));

    if (!ret) {
        LOG_FATAL("Could not post message {} to window {}: {}", Msg, reinterpret_cast<ptrdiff_t>(window), getLastErrorMessage());
    }
}

void application_win32::post_message(
    std::vector<void *> const &windows,
    unsigned int Msg,
    ptrdiff_t wParam,
    ptrdiff_t lParam) noexcept
{
    for (auto window : windows) {
        post_message(window, Msg, wParam, lParam);
    }
}

void application_win32::quit()
{
    run_from_main_loop([&]() {
        PostQuitMessage(0);
    });
}

void application_win32::init()
{
    application::init();

    languages_maintenance_callback = timer::global->add_callback(1s, [this](auto...) {
        ttlet current_language_tags = language::read_os_preferred_languages();
        static auto previous_language_tags = current_language_tags;
        
        if (previous_language_tags != current_language_tags) {
            previous_language_tags = current_language_tags;
            this->post_message(this->win32_windows(), WM_WIN_LANGUAGE_CHANGE);
        }
    });
}

int application_win32::loop()
{
    inLoop = true;

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

void application_win32::audioStart()
{
    application::audioStart();

    if (audio_system::global) {
        auto audio_system = std::dynamic_pointer_cast<audio_system_aggregate>(audio_system::global);
        audio_system->make_audio_system<audio_system_win32>();
    }
}

} // namespace tt
