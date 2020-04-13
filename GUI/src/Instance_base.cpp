// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Instance_base.hpp"
#include "TTauri/Foundation/logger.hpp"
#include <chrono>

namespace TTauri::GUI {

using namespace std;
using namespace gsl;

Device *Instance_base::findBestDeviceForWindow(Window const &window)
{
    std::scoped_lock lock(GUI_globals->mutex);

    int bestScore = -1;
    Device *bestDevice = nullptr;

    for (let &device : devices) {
        let score = device->score(window);
        LOG_INFO("Device has score={}.", score);

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

ssize_t Instance_base::getNumberOfWindows()
{
    std::scoped_lock lock(GUI_globals->mutex);

    ssize_t numberOfWindows = 0;
    for (const auto &device: devices) {
        numberOfWindows+= device->getNumberOfWindows();
    }

    return numberOfWindows;
}

void Instance_base::_handleVerticalSync(void *data, cpu_utc_clock::time_point displayTimePoint)
{
    auto self = static_cast<Instance_base *>(data);

    self->handleVerticalSync(displayTimePoint);
}


}
