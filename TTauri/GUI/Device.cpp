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

    windows.insert(window);
    window->setDevice(shared_from_this());
}

void Device::remove(std::shared_ptr<Window> window)
{
    std::scoped_lock lock(mutex);

    window->setDevice(nullptr);
    windows.erase(window);
}

bool Device::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    auto hasBlockedOnVSync = false;

    if (mutex.try_lock()) {
        if (state == State::READY_TO_DRAW) {

            for (auto window : windows) {
                hasBlockedOnVSync |= window->updateAndRender(nowTimestamp, outputTimestamp, blockOnVSync && !hasBlockedOnVSync);
            }
        }
        mutex.unlock();
    }

    return hasBlockedOnVSync;
}

void Device::maintance()
{
    for (auto window : windows) {
        window->maintenance();
    }
}

}}
