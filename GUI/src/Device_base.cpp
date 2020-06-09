// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Device_base.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/GUI/Window.hpp"
#include <fmt/format.h>
#include <tuple>
#include <vector>

namespace TTauri {

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
    auto lock = std::scoped_lock(guiMutex);

    return fmt::format("{0:04x}:{1:04x} {2} {3}", vendorID, deviceID, deviceName, deviceUUID.UUIDString());
}

void Device_base::initializeDevice(Window const &window)
{
    auto lock = std::scoped_lock(guiMutex);

    state = State::READY_TO_DRAW;
}

void Device_base::add(std::unique_ptr<Window> window)
{
    auto lock = std::scoped_lock(guiMutex);

    if (state == State::NO_DEVICE) {
        initializeDevice(*window);
    }

    auto _device = dynamic_cast<Device *>(this);
    ttauri_assert(_device);
    window->setDevice(_device);

    windows.push_back(std::move(window));
}

void Device_base::remove(Window &window) noexcept
{
    auto lock = std::scoped_lock(guiMutex);

    window.unsetDevice();
    windows.erase(std::find_if(windows.begin(), windows.end(), [&](auto &x) {
        return x.get() == &window;
    }));
}

}
