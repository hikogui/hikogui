// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system.hpp"
#include "gui_system_win32.hpp"
#include "../logger.hpp"
#include <chrono>

namespace tt {

using namespace std;

gui_window &gui_system::add_window(std::unique_ptr<gui_window> window)
{
    tt_axiom(is_gui_thread());

    auto device = gfx_system::global().findBestDeviceForSurface(*(window->surface));
    if (!device) {
        throw gui_error("Could not find a vulkan-device matching this window");
    }

    window->set_device(device);

    auto window_ptr = &(*window);
    _windows.push_back(std::move(window));
    return *window_ptr;
}

[[nodiscard]] gui_system *gui_system::subsystem_init() noexcept
{
    auto tmp = new gui_system_win32();
    tmp->init();
    return tmp;
}

void gui_system::subsystem_deinit() noexcept
{
    if (auto tmp = _global.exchange(nullptr)) {
        tmp->deinit();
        delete tmp;
    }
}

}
