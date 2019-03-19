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

Window::~Window()
{
}

void Window::initialize()
{
    view = TTauri::make_shared<WindowView>(shared_from_this());
    backingPipeline = TTauri::make_shared<BackingPipeline_vulkan>(shared_from_this());
    delegate->creatingWindow(shared_from_this());
}

void Window::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    if (mutex.try_lock()) {
        if (state == State::READY_TO_DRAW) {
            if (!render(blockOnVSync)) {
                LOG_INFO("Swapchain out of date.");
                state = State::SWAPCHAIN_OUT_OF_DATE;
            }
        }
        mutex.unlock();
    }
}

void Window::maintenance()
{
    std::scoped_lock lock(mutex);

    if (state == State::SWAPCHAIN_OUT_OF_DATE || state == State::MINIMIZED) {
        state = State::SWAPCHAIN_OUT_OF_DATE;

        auto onScreen = rebuildForSwapchainChange();

        state = onScreen ? State::READY_TO_DRAW : State::MINIMIZED;
    }
}

void Window::setDevice(const std::shared_ptr<Device> &device)
{
    if (device) {
        {
            std::scoped_lock lock(mutex);

            if (state == State::NO_DEVICE) {
                this->device = device;
                state = State::LINKED_TO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(Window::StateError());
            }
        }

        buildForDeviceChange();

    } else {
        teardownForDeviceChange();

        {
            std::scoped_lock lock(mutex);

            if (state == State::LINKED_TO_DEVICE) {
                this->device.reset();
                state = State::NO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(Window::StateError());
            }
        }
    }
}

}}
