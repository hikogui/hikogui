// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_win32.hpp"
#include "vertical_sync.hpp"
#include "../trace.hpp"

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
    std::optional<int> exit_code = {};

    // The first dead line is 15ms from now, in-time for the first
    // render to be performed at 60fps.
    auto display_time_point = hires_utc_clock::now();
    auto dead_line = display_time_point + 15ms;
    do {
        // Process messages from the queue until the dead line is reached.
        while (true) {
            MSG msg = {};
            if (not PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                // Event queue is empty, continue to render and vsync.
                break;
            }

            ttlet t = trace<"gui_system_event">();

            switch (msg.message) {
            case WM_APP_CALL_FUNCTION: {
                ttlet functionP = std::launder(reinterpret_cast<std::function<void()> *>(msg.lParam));
                (*functionP)();
                delete functionP;
            } break;

            case WM_QUIT: exit_code = narrow_cast<int>(msg.wParam); break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_NCLBUTTONDOWN) {
                // DispatchMessage will block when the user clicks the non-client area for
                // moving or resizing the window. Do not count these as missing the deadline.
                goto bypass_render;
            }

            if (hires_utc_clock::now() >= dead_line) {
                // dead line was passed while processing a message.
                increment_counter<"gui_system_event_dead_line">();
                break;
            }
        }

        // Render right after user input has been processed by the event queue.
        {
            ttlet t = trace<"gui_system_render">();
            render(display_time_point);

            if (hires_utc_clock::now() >= dead_line) {
                // dead line was passed while processing a message.
                increment_counter<"gui_system_render_dead_line">();
            }
        }

bypass_render:
        display_time_point = vertical_sync::global().wait();

        // The next dead line is 5ms before the current rendered frame is to be displayed.
        // But give the event loop at least 5ms to process messages.
        dead_line = std::max(hires_utc_clock::now() + 5ms, display_time_point - 5ms);

    } while (not exit_code);

    return *exit_code;
}

}