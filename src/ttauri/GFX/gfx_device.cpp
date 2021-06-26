// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_device.hpp"
#include "../GUI/gui_window.hpp"
#include <format>
#include <tuple>
#include <vector>

namespace tt {

using namespace std;

gfx_device::gfx_device(gfx_system &system) noexcept : system(system)
{
}

gfx_device::~gfx_device()
{
}

std::string gfx_device::string() const noexcept
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    return std::format("{0:04x}:{1:04x} {2} {3}", vendorID, deviceID, deviceName, deviceUUID.UUIDString());
}

}
