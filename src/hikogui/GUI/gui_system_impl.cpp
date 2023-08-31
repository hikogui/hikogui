// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system.hpp"
#include "gui_system_win32.hpp"
#include "keyboard_bindings.hpp"
#include "theme_book.hpp"
#include "../GFX/GFX.hpp"
#include "../telemetry/telemetry.hpp"
#include "../dispatch/dispatch.hpp"
#include "../macros.hpp"
#include <chrono>

namespace hi::inline v1 {

gui_system::gui_system(
    std::unique_ptr<hi::theme_book> theme_book,
    std::unique_ptr<hi::keyboard_bindings> keyboard_bindings,
    std::weak_ptr<gui_system_delegate> delegate) noexcept :
    theme_book(std::move(theme_book)),
    keyboard_bindings(std::move(keyboard_bindings)),
    thread_id(current_thread_id()),
    _delegate(delegate)
{
}

gui_system::~gui_system()
{
}

std::shared_ptr<gui_window> gui_system::add_window(std::shared_ptr<gui_window> window)
{
    hi_axiom(loop::main().on_thread());

    auto device = gfx_system::global().find_best_device_for_surface(window->surface->intrinsic);
    if (not device) {
        throw gui_error("Could not find a vulkan-device matching this window");
    }

    window->set_device(device);
    return std::move(window);
}

} // namespace hi::inline v1
