// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_win32.hpp"
#include "vertical_sync_win32.hpp"
#include "keyboard_bindings.hpp"
#include "theme_book.hpp"
#include "../GFX/gfx_system_vulkan.hpp"
#include "../text/font_book.hpp"
#include "../trace.hpp"

namespace tt {

[[nodiscard]] std::unique_ptr<gui_system> gui_system::make_unique(std::weak_ptr<gui_system_delegate> delegate) noexcept
{
    auto font_book = std::make_unique<tt::font_book>(std::vector<URL>{URL::urlFromSystemfontDirectory()});
    font_book->register_elusive_icon_font(URL("resource:elusiveicons-webfont.ttf"));
    font_book->register_ttauri_icon_font(URL("resource:ttauri_icons.ttf"));
    font_book->post_process();

    auto theme_book = std::make_unique<tt::theme_book>(*font_book, std::vector<URL>{URL::urlFromResourceDirectory() / "themes"});

    auto gfx_system = std::make_unique<tt::gfx_system_vulkan>();

    auto keyboard_bindings = std::make_unique<tt::keyboard_bindings>();
    try {
        keyboard_bindings->load_bindings(URL{"resource:win32.keybinds.json"}, true);
    } catch (std::exception const &e) {
        tt_log_fatal("Could not load keyboard bindings. \"{}\"", e.what());
    }

    auto r = std::make_unique<gui_system_win32>(
        std::move(gfx_system),
        std::make_unique<tt::vertical_sync_win32>(),
        std::move(font_book),
        std::move(theme_book),
        std::move(keyboard_bindings),
        std::move(delegate));
    r->init();
    return r;
}

gui_system_win32::gui_system_win32(
    std::unique_ptr<gfx_system> gfx,
    std::unique_ptr<tt::vertical_sync> vertical_sync,
    std::unique_ptr<tt::font_book> font_book,
    std::unique_ptr<tt::theme_book> theme_book,
    std::unique_ptr<tt::keyboard_bindings> keyboard_bindings,
    std::weak_ptr<gui_system_delegate> delegate) :
    gui_system(
        std::move(gfx),
        std::move(vertical_sync),
        std::move(font_book),
        std::move(theme_book),
        std::move(keyboard_bindings),
        std::move(delegate))
{
}

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
        display_time_point = vertical_sync->wait();

        // The next dead line is 5ms before the current rendered frame is to be displayed.
        // But give the event loop at least 5ms to process messages.
        dead_line = std::max(hires_utc_clock::now() + 5ms, display_time_point - 5ms);

    } while (not exit_code);

    return *exit_code;
}

} // namespace tt
