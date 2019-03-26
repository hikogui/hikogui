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

namespace TTauri { namespace GUI {

using namespace std;

Window::Window(const std::shared_ptr<Delegate> &delegate, const std::string &title) :
    state(State::NO_DEVICE),
    delegate(delegate),
    title(title)
{
}

void Window::initialize()
{
    view = TTauri::make_shared<WindowView>(shared_from_this());
    backingPipeline = TTauri::make_shared<BackingPipeline_vulkan>(shared_from_this());
    delegate->creatingWindow(shared_from_this());
}

bool Window::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    return render(blockOnVSync);
}

void Window::maintenance()
{
    LOG_DEBUG("maintenance");
    if (state.try_transition({{State::SWAPCHAIN_OUT_OF_DATE, State::REBUILDING_SWAPCHAIN}, {State::MINIMIZED, State::REBUILDING_SWAPCHAIN}})) {
        auto const newState = rebuildForSwapchainChange();

        state.transition_or_throw({{State::REBUILDING_SWAPCHAIN, newState}});
    }
}

void Window::setDevice(const std::shared_ptr<Device> &device)
{
    state.transition({
        {State::READY_TO_DRAW, State::SETTING_DEVICE},
        {State::NO_DEVICE, State::SETTING_DEVICE},
        {State::MINIMIZED, State::SETTING_DEVICE},
        {State::SWAPCHAIN_OUT_OF_DATE, State::SETTING_DEVICE},

        {State::WAITING_FOR_VSYNC, State::REQUEST_SET_DEVICE}
    });

    state.transition({
        {State::SETTING_DEVICE, State::SETTING_DEVICE},
        {State::ACCEPTED_SET_DEVICE, State::SETTING_DEVICE}
    });

    auto const oldDevice = this->device.lock();

    if (oldDevice) {
        teardownForDeviceChange();
        this->device.reset();
    }

    if (device) {
        this->device = device;
        auto const newState = buildForDeviceChange();
        state.transition_or_throw({{State::SETTING_DEVICE, newState}});

    } else {
        state.transition_or_throw({{State::SETTING_DEVICE, State::NO_DEVICE}});
    }   
}

void Window::setWindowPosition(uint32_t x, uint32_t y)
{
    //std::scoped_lock lock(mutex);

    windowRectangle.offset = {x, y};
}

void Window::setWindowSize(uint32_t width, uint32_t height)
{
    //std::scoped_lock lock(mutex);

    windowRectangle.extent = {width, height};
}

}}
