// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system.hpp"
#include "gui_system_win32.hpp"
#include "keyboard_bindings.hpp"
#include "theme_book.hpp"
#include "vertical_sync.hpp"
#include "../GFX/gfx_system.hpp"
#include "../text/font_book.hpp"
#include "../log.hpp"
#include "../os_settings.hpp"
#include <chrono>

namespace tt::inline v1 {

gui_system::gui_system(
    std::shared_ptr<tt::event_queue> event_queue,
    std::unique_ptr<gfx_system> gfx,
    std::unique_ptr<tt::vertical_sync> vertical_sync,
    std::unique_ptr<tt::font_book> font_book,
    std::unique_ptr<tt::theme_book> theme_book,
    std::unique_ptr<tt::keyboard_bindings> keyboard_bindings,
    std::weak_ptr<gui_system_delegate> delegate) noexcept :
    _event_queue(std::move(event_queue)),
    gfx(std::move(gfx)),
    vertical_sync(std::move(vertical_sync)),
    font_book(std::move(font_book)),
    theme_book(std::move(theme_book)),
    keyboard_bindings(std::move(keyboard_bindings)),
    thread_id(current_thread_id()),
    _delegate(delegate)
{
    this->gfx->init();
}

gui_system::~gui_system()
{
    gfx->deinit();
}

gui_window &gui_system::add_window(std::unique_ptr<gui_window> window)
{
    tt_axiom(is_gui_thread());

    auto device = gfx->find_best_device_for_surface(*(window->surface));
    if (!device) {
        throw gui_error("Could not find a vulkan-device matching this window");
    }

    window->set_device(device);

    auto window_ptr = &(*window);
    _windows.push_back(std::move(window));
    return *window_ptr;
}

void gui_system::request_reconstrain() noexcept
{
    for (auto &window : _windows) {
        window->request_reconstrain();
    }
}

} // namespace tt::inline v1
