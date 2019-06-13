// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_base.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "WindowWidget.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri::GUI {

using namespace std;

Window_base::Window_base(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    state(State::INITIALIZING),
    delegate(move(delegate)),
    title(move(title))
{
}

Window_base::~Window_base()
{
    try {
        [[gsl::suppress(f.6)]] {
            if (state != State::NO_WINDOW) {
                LOG_FATAL("Window was not properly teardown before destruction.") % title;
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
    Window *thisWindow = dynamic_cast<Window *>(this);
    assert(thisWindow);
    delegate->openingWindow(*thisWindow);
    state = State::NO_DEVICE;
}

void Window_base::closingWindow() {
    Window* thisWindow = dynamic_cast<Window*>(this);
    assert(thisWindow);
    delegate->closingWindow(*thisWindow);
    state = State::NO_WINDOW;
}

void Window_base::setDevice(const std::weak_ptr<Device> newDevice)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (!device.expired()) {
        state = State::DEVICE_LOST;
        teardown();
    }

    device = newDevice;
}



}
