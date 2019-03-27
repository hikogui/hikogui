//
//  Device.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Device.hpp"

#include "Instance.hpp"
#include "Window.hpp"

#include "TTauri/Logging.hpp"
#include "TTauri/utils.hpp"


#include <boost/uuid/uuid_io.hpp>

#include <tuple>
#include <vector>

namespace TTauri {
namespace GUI {

using namespace std;


Device::Device()
{
}

Device::~Device()
{
    windows.clear();
}

std::string Device::str() const
{
    return (boost::format("%04x:%04x %s %s") % vendorID % deviceID % deviceName % deviceUUID).str();
}

void Device::initializeDevice(std::shared_ptr<Window> window)
{
    state = State::READY_TO_DRAW;
}

void Device::add(std::shared_ptr<Window> window)
{
    if (state == State::NO_DEVICE) {
        initializeDevice(window);
    }

    std::scoped_lock lock(mutex);

    windows.push_back(window);
    window->setDevice(shared_from_this());
}

void Device::remove(std::shared_ptr<Window> window)
{
    std::scoped_lock lock(mutex);

    window->setDevice(nullptr);
    windows.erase(find(windows.begin(), windows.end(), window));
}

bool Device::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    vector<shared_ptr<Window>> tmpWindows;
    {
        auto lock = scoped_lock(mutex);
        tmpWindows = windows;
    }

    auto hasBlockedOnVSync = false;

    if (state == State::READY_TO_DRAW) {
        for (auto window : tmpWindows) {
            hasBlockedOnVSync |= window->updateAndRender(nowTimestamp, outputTimestamp, blockOnVSync && !hasBlockedOnVSync);
        }
    }

    return hasBlockedOnVSync;
}

std::vector<std::shared_ptr<Window>> Device::maintance()
{
    vector<shared_ptr<Window>> tmpWindows;
    vector<shared_ptr<Window>> orphanWindows;

    {
        auto lock = scoped_lock(mutex);
        tmpWindows = windows;
    }

    for (auto window : tmpWindows) {
        if (window->hasLostSurface()) {
            // Window must be destroyed.
            window->closingWindow();
            remove(window);

        } else if (window->hasLostDevice()) {
            // Window must be passed to the Instance, for reinsertion on a new device.
            remove(window);
            orphanWindows.push_back(window);

        } else {
            window->maintenance();

        }
    }

    return orphanWindows;
}

}}
