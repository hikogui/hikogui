// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_base.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "WindowWidget.hpp"
#include "TTauri/all.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri::GUI {

using namespace std;

Window_base::Window_base(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    state(State::NO_DEVICE),
    delegate(move(delegate)),
    title(move(title))
{
}

Window_base::~Window_base()
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

void Window_base::initialize()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    auto window = std::dynamic_pointer_cast<Window>(shared_from_this());
    widget = TTauri::make_shared<WindowWidget>(window);

    openingWindow();
}

void Window_base::openingWindow() {
    delegate->openingWindow(dynamic_cast<Window &>(*this));
}

void Window_base::closingWindow() {
    delegate->closingWindow(dynamic_cast<Window&>(*this));
}

void Window_base::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    render();
}

void Window_base::maintenance()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (state == State::SWAPCHAIN_OUT_OF_DATE || state == State::MINIMIZED) {
        state = rebuildForSwapchainChange();
    }
}

void Window_base::setDevice(const std::weak_ptr<Device> newDevice)
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

void Window_base::setWindowPosition(uint32_t x, uint32_t y)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    //windowRectangle.offset = {x, y};
}

void Window_base::setWindowSize(uint32_t width, uint32_t height)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    //windowRectangle.extent = {width, height};
}

void Window_base::windowChangedSize(u64extent2 extent) {
    if (widthHeightContraintsAdded) {
        removeConstraint(widthConstraint);
        removeConstraint(heightConstraint);
    }
    widthConstraint = (box().width == extent.width());
    heightConstraint = (box().height == extent.height());
    addConstraint(widthConstraint);
    addConstraint(heightConstraint);
    widthHeightContraintsAdded = true;
}


}
