// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system.hpp"
#include "gui_system_win32.hpp"
#include "keyboard_bindings.hpp"
#include "theme_book.hpp"
#include "../GFX/gfx_system.hpp"
#include "../text/font_book.hpp"
#include "../log.hpp"
#include "../loop.hpp"
#include <chrono>

namespace hi::inline v1 {

gui_system::gui_system(
    std::unique_ptr<gfx_system> gfx,
    std::unique_ptr<hi::font_book> font_book,
    std::unique_ptr<hi::theme_book> theme_book,
    std::unique_ptr<hi::keyboard_bindings> keyboard_bindings,
    std::weak_ptr<gui_system_delegate> delegate) noexcept :
    gfx(std::move(gfx)),
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

std::shared_ptr<gui_window> gui_system::add_window(std::shared_ptr<gui_window> window)
{
    hi_axiom(loop::main().on_thread());

    auto device = gfx->find_best_device_for_surface(*(window->surface));
    if (!device) {
        throw gui_error("Could not find a vulkan-device matching this window");
    }

    window->set_device(device);
    loop::main().add_window(window);
    return std::move(window);
}

} // namespace hi::inline v1
