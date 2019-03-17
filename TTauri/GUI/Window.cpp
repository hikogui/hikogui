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

#include "TTauri/Logging.hpp"

#include <boost/numeric/conversion/cast.hpp>

namespace TTauri { namespace GUI {

using namespace std;

#pragma mark "Public"
// Public methods need to lock the instance.

Window::Window(std::shared_ptr<Delegate> delegate, const std::string &title) :
    state(State::NO_DEVICE),
    delegate(delegate),
    title(title)
{
    view = std::make_shared<WindowView>(this);
    backingPipeline = std::make_shared<BackingPipeline>(this);
}

Window::~Window() {}

void Window::initialize()
{
    delegate->initialize(this);
}

void Window::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    if (stateMutex.try_lock()) {
        if (state == State::READY_TO_DRAW) {
            if (!render(blockOnVSync)) {
                LOG_INFO("Swapchain out of date.");
                state = State::SWAPCHAIN_OUT_OF_DATE;
            }
        }
        stateMutex.unlock();
    }
}

void Window::maintenance()
{
    std::scoped_lock lock(stateMutex);

    if (state == State::SWAPCHAIN_OUT_OF_DATE || state == State::MINIMIZED) {
        state = State::SWAPCHAIN_OUT_OF_DATE;

        auto onScreen = rebuildForSwapchainChange();

        state = onScreen ? State::READY_TO_DRAW : State::MINIMIZED;
    }
}



void Window::setDevice(Device *device)
{
    if (device) {
        {
            std::scoped_lock lock(stateMutex);

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
            std::scoped_lock lock(stateMutex);

            if (state == State::LINKED_TO_DEVICE) {
                this->device = nullptr;
                state = State::NO_DEVICE;

            } else {
                BOOST_THROW_EXCEPTION(Window::StateError());
            }
        }
    }
}

#pragma mark "Private"
// Private methods don't need to lock.




}}
