// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_win32.hpp"

namespace tt {

// WM_USER = ?-0x7fff
// WM_APP = 0x8000-0xbfff.
constexpr unsigned int WM_APP_CALL_FUNCTION = 0x8000 + 1;

void gui_system_win32::run_from_event_queue(std::function<void()> function)
{
    ttlet functionP = new std::function<void()>(std::move(function));
    tt_assert(functionP);

    auto r = PostThreadMessageW(thread_id, WM_APP_CALL_FUNCTION, 0, reinterpret_cast<LPARAM>(functionP));
    tt_assert(r != 0);
}

void gui_system_win32::exit(int exit_code)
{
    run_from_event_queue([exit_code]() {
        PostQuitMessage(exit_code);
    });
}

int gui_system_win32::loop()
{
    // Run the message loop.
    int exit_code = 0;
    MSG msg = {};

    BOOL more_messages;
    do {
        more_messages = GetMessage(&msg, nullptr, 0, 0);
        switch (msg.message) {
        case WM_APP_CALL_FUNCTION: {
            ttlet functionP = reinterpret_cast<std::function<void()> *>(msg.lParam);
            (*functionP)();
            delete functionP;
        } break;

        case WM_QUIT: exit_code = narrow_cast<int>(msg.wParam); break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    } while (more_messages);

    return exit_code;
}

}