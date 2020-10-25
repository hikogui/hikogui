// Copyright 2019 Pokitec
// All rights reserved.

#include "GUISystem_base.hpp"
#include "../logger.hpp"
#include <chrono>

namespace tt {

using namespace std;

GUIDevice *GUISystem_base::findBestDeviceForWindow(Window const &window)
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    int bestScore = -1;
    GUIDevice *bestDevice = nullptr;

    for (ttlet &device : devices) {
        ttlet score = device->score(window);
        LOG_INFO("GUIDevice has score={}.", score);

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

ssize_t GUISystem_base::getNumberOfWindows()
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    ssize_t numberOfWindows = 0;
    for (const auto &device: devices) {
        numberOfWindows+= device->getNumberOfWindows();
    }

    return numberOfWindows;
}

void GUISystem_base::_handleVerticalSync(void *data, hires_utc_clock::time_point displayTimePoint)
{
    auto self = static_cast<GUISystem_base *>(data);

    self->handleVerticalSync(displayTimePoint);
}


}
