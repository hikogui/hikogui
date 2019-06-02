// Copyright 2019 Pokitec
// All rights reserved.

#include "Device_base.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "Window.hpp"
#include "TTauri/all.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <tuple>
#include <vector>

namespace TTauri::GUI {

using namespace std;

Device_base::Device_base()
{
}

Device_base::~Device_base()
{
    windows.clear();
}

std::string Device_base::str() const
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    return (boost::format("%04x:%04x %s %s") % vendorID % deviceID % deviceName % deviceUUID).str();
}

void Device_base::initializeDevice(std::shared_ptr<Window> window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);
    
    state = State::READY_TO_DRAW;
}

void Device_base::add(std::shared_ptr<Window> window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (state == State::NO_DEVICE) {
        initializeDevice(window);
    }

    windows.push_back(window);

    auto device = std::dynamic_pointer_cast<Device>(shared_from_this());
    assert(device);
    window->setDevice(device);
}

void Device_base::remove(std::shared_ptr<Window> window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    window->unsetDevice();
    windows.erase(find(windows.begin(), windows.end(), window));
}

std::vector<std::shared_ptr<Window>> Device_base::maintance()
{
    auto lock = scoped_lock(TTauri::GUI::mutex);

    auto tmpWindows = windows;
    decltype(tmpWindows) orphanWindows;

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

}
