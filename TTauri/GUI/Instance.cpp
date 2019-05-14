// Copyright 2019 Pokitec
// All rights reserved.

#include "Instance.hpp"
#include "TTauri/logging.hpp"
#include "TTauri/Application.hpp"
#include <chrono>

namespace TTauri::GUI {

using namespace std;
using namespace gsl;

std::shared_ptr<Instance> Instance::singleton = nullptr;


shared_ptr<Device> Instance::findBestDeviceForWindow(const shared_ptr<Window> &window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    int bestScore = -1;
    shared_ptr<Device> bestDevice;

    for (auto const &device : devices) {
        auto const score = device->score(window);
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

void Instance::add(shared_ptr<Window> window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto device = findBestDeviceForWindow(window);
    if (!device) {
        BOOST_THROW_EXCEPTION(ErrorNoDeviceForWindow());
    }

    device->add(window);
    verticalSync.add(window);
}

size_t Instance::getNumberOfWindows()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    size_t numberOfWindows = 0;
    for (const auto &device: devices) {
        numberOfWindows+= device->windows.size();
    }

    return numberOfWindows;
}

void Instance::maintenance()
{
    scoped_lock lock(TTauri::GUI::mutex);

    auto tmpDevices = devices;

    // Check how many windows are still open.
    auto numberOfOpenWindowsBefore = getNumberOfWindows();

    for (auto device : tmpDevices) {
        auto orphanWindow = device->maintance();
    }

    // Check how many windows are still open.
    auto numberOfOpenWindowsAfter = getNumberOfWindows();

    if (numberOfOpenWindowsAfter == 0 && numberOfOpenWindowsBefore > 0) {
        get_singleton<Application>()->lastWindowClosed();
    }
}


}
