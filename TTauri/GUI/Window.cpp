// Copyright 2019 Pokitec
// All rights reserved.

#include "Window.hpp"
#include "Device.hpp"
#include "WindowWidget.hpp"
#include "TTauri/all.hpp"
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
    std::scoped_lock lock(TTauri::GUI::mutex);

    widget = TTauri::make_shared<WindowWidget>(shared_from_this());

    openingWindow();
}

void Window::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    render();
}

void Window::maintenance()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (state == State::SWAPCHAIN_OUT_OF_DATE || state == State::MINIMIZED) {
        state = rebuildForSwapchainChange();
    }
}

void Window::setDevice(const std::weak_ptr<Device> newDevice)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (!device.expired()) {
        teardownForDeviceChange();
    }

    device = move(newDevice);
    if (!device.expired()) {
        state = buildForDeviceChange();
    } else {
        state = State::NO_DEVICE;
    }   
}

void Window::setWindowPosition(uint32_t x, uint32_t y)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    //windowRectangle.offset = {x, y};
}

void Window::setWindowSize(uint32_t width, uint32_t height)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    //windowRectangle.extent = {width, height};
}

}
