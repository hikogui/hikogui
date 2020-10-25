// Copyright 2019 Pokitec
// All rights reserved.

#include "GUIDevice_base.hpp"
#include "GUIDevice.hpp"
#include "GUISystem.hpp"
#include "Window.hpp"
#include <fmt/format.h>
#include <tuple>
#include <vector>

namespace tt {

using namespace std;

GUIDevice_base::GUIDevice_base(GUISystem &system) noexcept : system(system)
{
}

GUIDevice_base::~GUIDevice_base()
{
    windows.clear();
}

std::string GUIDevice_base::string() const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    return fmt::format("{0:04x}:{1:04x} {2} {3}", vendorID, deviceID, deviceName, deviceUUID.UUIDString());
}

void GUIDevice_base::initializeDevice(Window const &window)
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    state = State::READY_TO_DRAW;
}

void GUIDevice_base::add(std::shared_ptr<Window> window)
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    if (state == State::NO_DEVICE) {
        initializeDevice(*window);
    }

    auto _device = dynamic_cast<GUIDevice *>(this);
    tt_assert(_device);
    window->setDevice(_device);

    windows.push_back(std::move(window));
}

void GUIDevice_base::remove(Window &window) noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    window.unsetDevice();
    windows.erase(std::find_if(windows.begin(), windows.end(), [&](auto &x) {
        return x.get() == &window;
    }));
}

}
