//
//  Window.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Window.hpp"

#include "Device.hpp"
#include "WindowView.hpp"
#include "config.hpp"

#include "TTauri/utils.hpp"
#include "TTauri/Logging.hpp"

#include <boost/numeric/conversion/cast.hpp>

namespace TTauri::GUI {

using namespace std;

Window::Window(const std::shared_ptr<Delegate> delegate, const std::string title) :
    state(State::NO_DEVICE),
    delegate(move(delegate)),
    title(move(title))
{
}

Window::~Window()
{
    try {
        [[gsl::suppress(f.6)]] {
            if (state != State::NO_DEVICE || !this->device.expired()) {
                LOG_FATAL("Device was associated with Window '%s' during destruction of the Window.") % title;
                abort();
            }
            LOG_INFO("Window '%s' has been propertly destructed.") % title;
        }
    } catch (...) {
        abort();
    }
}

void Window::initialize()
{
    view = TTauri::make_shared<WindowView>(shared_from_this());

    openingWindow();
}

bool Window::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    return render(blockOnVSync);
}

void Window::maintenance()
{
    if (state.try_transition({{State::SWAPCHAIN_OUT_OF_DATE, State::REBUILDING_SWAPCHAIN}, {State::MINIMIZED, State::REBUILDING_SWAPCHAIN}})) {
        auto const newState = rebuildForSwapchainChange();

        state.transition_or_throw({{State::REBUILDING_SWAPCHAIN, newState}});
    }
}

void Window::setDevice(const std::weak_ptr<Device> newDevice)
{
    state.transition({
        {State::READY_TO_DRAW, State::SETTING_DEVICE},
        {State::NO_DEVICE, State::SETTING_DEVICE},
        {State::MINIMIZED, State::SETTING_DEVICE},
        {State::SWAPCHAIN_OUT_OF_DATE, State::SETTING_DEVICE},
        {State::DEVICE_LOST, State::SETTING_DEVICE},
        {State::SURFACE_LOST, State::SETTING_DEVICE},

        {State::WAITING_FOR_VSYNC, State::REQUEST_SET_DEVICE}
    });

    state.transition({
        {State::SETTING_DEVICE, State::SETTING_DEVICE},
        {State::ACCEPTED_SET_DEVICE, State::SETTING_DEVICE}
    });

    if (!device.expired()) {
        teardownForDeviceChange();
    }

    device = move(newDevice);
    if (!device.expired()) {
        auto const newState = buildForDeviceChange();
        state.transition_or_throw({{State::SETTING_DEVICE, newState}});

    } else {
        state.transition_or_throw({{State::SETTING_DEVICE, State::NO_DEVICE}});
    }   
}

void Window::setWindowPosition(uint32_t x, uint32_t y)
{
    windowRectangle.offset = {x, y};
}

void Window::setWindowSize(uint32_t width, uint32_t height)
{
    windowRectangle.extent = {width, height};
}

}
