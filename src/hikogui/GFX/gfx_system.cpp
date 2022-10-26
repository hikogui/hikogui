// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_system.hpp"
#include "gfx_system_vulkan.hpp"
#include "gfx_surface.hpp"
#include "../log.hpp"
#include <chrono>

namespace hi::inline v1 {

gfx_system::gfx_system() noexcept {}

gfx_system::~gfx_system() {}

gfx_device *gfx_system::find_best_device_for_surface(gfx_surface const &surface)
{
    hilet lock = std::scoped_lock(gfx_system_mutex);

    int best_score = -1;
    gfx_device *best_device = nullptr;

    for (hilet &device : devices) {
        hilet score = device->score(surface);
        if (score >= best_score) {
            best_score = score;
            best_device = device.get();
        }
    }

    if (best_score <= 0) {
        hi_log_fatal("Could not find a graphics device suitable for presenting this window.");
    }
    return best_device;
}

} // namespace hi::inline v1
