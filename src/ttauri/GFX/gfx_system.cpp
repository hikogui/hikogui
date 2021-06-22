// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gfx_system.hpp"
#include "gfx_system_vulkan.hpp"
#include "gfx_surface.hpp"
#include "../logger.hpp"
#include <chrono>

namespace tt {

using namespace std;

gfx_device *gfx_system::findBestDeviceForSurface(gfx_surface const &surface)
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);

    int bestScore = -1;
    gfx_device *bestDevice = nullptr;

    for (ttlet &device : devices) {
        ttlet score = device->score(surface);
        tt_log_info("gfx_device has score={}.", score);

        if (score >= bestScore) {
            bestScore = score;
            bestDevice = device.get();
        }
    }

    switch (bestScore) {
    case -1:
        return nullptr;
    case 0:
        fprintf(stderr, "Could not really find a device that can present this window.");
        /* FALLTHROUGH */
    default:
        return bestDevice;
    }
}

[[nodiscard]] gfx_system *gfx_system::subsystem_init() noexcept
{
    auto tmp = new gfx_system_vulkan();
    tmp->init();
    return tmp;
}

[[nodiscard]] void gfx_system::subsystem_deinit() noexcept
{
    if (auto tmp = _global.exchange(nullptr)) {
        tmp->deinit();
        delete tmp;
    }
}



}
