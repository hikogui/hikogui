// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_device.hpp"
#include "gui_window.hpp"
#include <fmt/format.h>
#include <tuple>
#include <vector>

namespace tt {

using namespace std;

gui_device::gui_device(gui_system &system) noexcept : system(system)
{
}

gui_device::~gui_device()
{
    windows.clear();
}

std::string gui_device::string() const noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    return fmt::format("{0:04x}:{1:04x} {2} {3}", vendorID, deviceID, deviceName, deviceUUID.UUIDString());
}

void gui_device::initialize_device(gui_window const &window)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    state = state_type::ready_to_draw;
}

void gui_device::add(std::shared_ptr<gui_window> window)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    if (state == state_type::no_device) {
        initialize_device(*window);
    }

    window->set_device(this);
    windows.push_back(std::move(window));
}

void gui_device::remove(gui_window &window) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    window.set_device(nullptr);
    windows.erase(std::find_if(windows.begin(), windows.end(), [&](auto &x) {
        return x.get() == &window;
    }));
}

}
