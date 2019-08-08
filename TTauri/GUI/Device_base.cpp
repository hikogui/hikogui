// Copyright 2019 Pokitec
// All rights reserved.

#include "Device_base.hpp"
#include "Device.hpp"
#include "Instance.hpp"
#include "Window.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <tuple>
#include <vector>

namespace TTauri::GUI {

using namespace std;

Device_base::Device_base() noexcept
{
}

Device_base::~Device_base()
{
    windows.clear();
}

std::string Device_base::string() const noexcept
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    return (boost::format("%04x:%04x %s %s") % vendorID % deviceID % deviceName % deviceUUID).str();
}

void Device_base::initializeDevice(Window const &window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);
    
    state = State::READY_TO_DRAW;
}

void Device_base::add(std::unique_ptr<Window> window)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (state == State::NO_DEVICE) {
        initializeDevice(*window);
    }

    auto _device = dynamic_cast<Device *>(this);
    required_assert(_device);
    window->setDevice(_device);

    windows.push_back(std::move(window));
}

void Device_base::remove(Window &window) noexcept
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    window.unsetDevice();
    windows.erase(std::find_if(windows.begin(), windows.end(), [&](auto &x) {
        return x.get() == &window;
    }));
}

}
