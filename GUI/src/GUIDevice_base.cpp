// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/GUIDevice_base.hpp"
#include "TTauri/GUI/GUIDevice.hpp"
#include "TTauri/GUI/GUISystem.hpp"
#include "TTauri/GUI/Window.hpp"
#include <fmt/format.h>
#include <tuple>
#include <vector>

namespace TTauri {

using namespace std;

GUIDevice_base::GUIDevice_base() noexcept
{
}

GUIDevice_base::~GUIDevice_base()
{
    windows.clear();
}

std::string GUIDevice_base::string() const noexcept
{
    auto lock = std::scoped_lock(guiMutex);

    return fmt::format("{0:04x}:{1:04x} {2} {3}", vendorID, deviceID, deviceName, deviceUUID.UUIDString());
}

void GUIDevice_base::initializeDevice(Window const &window)
{
    auto lock = std::scoped_lock(guiMutex);

    state = State::READY_TO_DRAW;
}

void GUIDevice_base::add(std::unique_ptr<Window> window)
{
    auto lock = std::scoped_lock(guiMutex);

    if (state == State::NO_DEVICE) {
        initializeDevice(*window);
    }

    auto _device = dynamic_cast<GUIDevice *>(this);
    ttauri_assert(_device);
    window->setDevice(_device);

    windows.push_back(std::move(window));
}

void GUIDevice_base::remove(Window &window) noexcept
{
    auto lock = std::scoped_lock(guiMutex);

    window.unsetDevice();
    windows.erase(std::find_if(windows.begin(), windows.end(), [&](auto &x) {
        return x.get() == &window;
    }));
}

}
