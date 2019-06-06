// Copyright 2019 Pokitec
// All rights reserved.

#include "Instance_base.hpp"
#include "TTauri/all.hpp"
#include <chrono>

namespace TTauri::GUI {

using namespace std;
using namespace gsl;

shared_ptr<Device> Instance_base::findBestDeviceForWindow(const shared_ptr<Window> &window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    int bestScore = -1;
    shared_ptr<Device> bestDevice;

    for (let &device : devices) {
        let score = device->score(window);
        LOG_INFO("Device has score=%i.") % score;

        if (score >= bestScore) {
            bestScore = score;
            bestDevice = device;
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

void Instance_base::add(shared_ptr<Window> window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto device = findBestDeviceForWindow(window);
    if (!device) {
        BOOST_THROW_EXCEPTION(ErrorNoDeviceForWindow());
    }

    device->add(window);
}

size_t Instance_base::getNumberOfWindows()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    size_t numberOfWindows = 0;
    for (const auto &device: devices) {
        numberOfWindows+= device->getNumberOfWindows();
    }

    return numberOfWindows;
}

void Instance_base::_handleVerticalSync(void *data)
{
    auto self = static_cast<Instance_base *>(data);

    self->handleVerticalSync();
}


}
