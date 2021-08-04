// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system.hpp"
#include "gui_system_win32.hpp"
#include "../logger.hpp"
#include <chrono>

namespace tt {

gui_system::gui_system(
    std::unique_ptr<gfx_system> gfx,
    std::unique_ptr<vertical_sync> vsync,
    std::unique_ptr<theme_book> themes,
    std::weak_ptr<gui_system_delegate> delegate) noexcept :
    gfx(std::move(gfx)), vsync(std::move(vsync)), themes(std::move(themes)), thread_id(current_thread_id()), _delegate(delegate)
{
    this->gfx->init();
    set_theme(this->themes->find("default", read_os_theme_mode()));
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

void gui_system::request_constrain() noexcept
{
    for (auto &window: _windows) {
        window->request_constrain = true;
    }
}

} // namespace tt
