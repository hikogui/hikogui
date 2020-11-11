// Copyright 2019 Pokitec
// All rights reserved.

#include "gui_system.hpp"
#include "../logger.hpp"
#include <chrono>

namespace tt {

using namespace std;

gui_device *gui_system::findBestDeviceForWindow(Window_base const &window)
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    int bestScore = -1;
    gui_device *bestDevice = nullptr;

    for (ttlet &device : devices) {
        ttlet score = device->score(window);
        LOG_INFO("gui_device has score={}.", score);

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

ssize_t gui_system::getNumberOfWindows()
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    ssize_t numberOfWindows = 0;
    for (const auto &device: devices) {
        numberOfWindows+= device->getNumberOfWindows();
    }

    return numberOfWindows;
}

void gui_system::_handleVerticalSync(void *data, hires_utc_clock::time_point displayTimePoint)
{
    auto self = static_cast<gui_system *>(data);

    self->handleVerticalSync(displayTimePoint);
}


}
