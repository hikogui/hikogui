// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system.hpp"
#include "../logger.hpp"
#include <chrono>

namespace tt {

using namespace std;

gui_device *gui_system::findBestDeviceForWindow(gui_window const &window)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    int bestScore = -1;
    gui_device *bestDevice = nullptr;

    for (ttlet &device : devices) {
        ttlet score = device->score(window);
        tt_log_info("gui_device has score={}.", score);

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

ssize_t gui_system::num_windows()
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    ssize_t numberOfWindows = 0;
    for (const auto &device: devices) {
        numberOfWindows+= device->num_windows();
    }

    return numberOfWindows;
}

void gui_system::_handlevertical_sync(void *data, hires_utc_clock::time_point displayTimePoint)
{
    auto self = static_cast<gui_system *>(data);

    self->handlevertical_sync(displayTimePoint);
}


}
