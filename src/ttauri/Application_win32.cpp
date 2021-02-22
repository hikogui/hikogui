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

application_win32::application_win32(
    std::weak_ptr<application_delegate> const &delegate,
    std::vector<std::string> const &arguments,
    os_handle instance) :
    application(delegate, arguments, instance)
{
}

void application_win32::run_from_main_loop(std::function<void()> function)
{
    tt_assert(inLoop);

    ttlet functionP = new std::function<void()>(std::move(function));
    tt_assert(functionP);

    auto r = PostThreadMessageW(main_thread_id, WM_APP_CALL_FUNCTION, 0, reinterpret_cast<LPARAM>(functionP));
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
    EnumThreadWindows(main_thread_id, win32_windows_EnumThreadWndProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

void application_win32::post_message(void *window, unsigned int Msg, ptrdiff_t wParam, ptrdiff_t lParam) noexcept
{
    ttlet ret = PostMessageW(
        reinterpret_cast<HWND>(window), static_cast<UINT>(Msg), static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));

    if (!ret) {
        tt_log_fatal("Could not post message {} to window {}: {}", Msg, reinterpret_cast<ptrdiff_t>(window), get_last_error_message());
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

void application_win32::exit(int exit_code)
{
    run_from_main_loop([exit_code]() {
        PostQuitMessage(exit_code);
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
    int exit_code = 0;
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        switch (msg.message) {
        case WM_APP_CALL_FUNCTION: {
            ttlet functionP = reinterpret_cast<std::function<void()> *>(msg.lParam);
            (*functionP)();
            delete functionP;
        } break;

        case WM_QUIT:
            exit_code = narrow_cast<int>(msg.wParam);
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    inLoop = false;
    return exit_code;
}

void application_win32::init_audio()
{
    application::init_audio();

    if (audio_system::global) {
        auto audio_system = std::dynamic_pointer_cast<audio_system_aggregate>(audio_system::global);
        audio_system->make_audio_system<audio_system_win32>();
    }
}

} // namespace tt
