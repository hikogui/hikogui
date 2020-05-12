// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Window_base.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/GUI/Widget.hpp"

namespace TTauri::GUI {

using namespace std;

Window_base::Window_base(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    state(State::Initializing),
    delegate(move(delegate)),
    title(move(title))
{
}

Window_base::~Window_base()
{
    // Destroy the top-level widget, before automatic destruction of the constraint solver
    // and other objects that the widgets require from the window during their destruction.
    widget.release();

    try {
        gsl_suppress(f.6) {
            if (state != State::NoWindow) {
                LOG_FATAL("Window '{}' was not properly teardown before destruction.", title);
            }
            LOG_INFO("Window '{}' has been propertly destructed.", title);
        }
    } catch (...) {
        abort();
    }
}

void Window_base::initialize()
{
    auto lock = std::scoped_lock(guiMutex);

    widget = Widgets::Widget::make_unique_WindowWidget(*static_cast<Window *>(this));

    // The width and height of the window will be modified by the user and also for
    // testing the minimum and maximum size of the window.
    widgetSolver.add_stay(widget->box.width, rhea::strength::medium());
    widgetSolver.add_stay(widget->box.height, rhea::strength::medium());

    openingWindow();
}

void Window_base::render(cpu_utc_clock::time_point displayTimePoint)
{
    layout();
}

void Window_base::layout()
{
    auto changed = layoutChildren(forceLayout.exchange(false));
    if (changed) {
        calculateMinimumAndMaximumWindowExtent();
    }
}

bool Window_base::layoutChildren(bool force) {
    constexpr int layout_retries = 10;

    auto changed = false;

    for (auto i = 0; i != layout_retries; ++i) {
        bool widget_changed = false;
        if (force || widget->needsLayout()) {
            widget_changed = widget->layout();
        }

        // Grandchildren need to be layed out when the child has changed.
        changed = widget->layoutChildren(force || widget_changed);

        if (!changed) {
            return i != 0;
        }
    }
    LOG_FATAL("Unable to layout child widgets");
}

void Window_base::openingWindow() {
    Window *thisWindow = dynamic_cast<Window *>(this);
    assert(thisWindow);
    delegate->openingWindow(*thisWindow);

    auto lock = std::scoped_lock(guiMutex);
    state = State::NoDevice;
    updateToNextKeyboardTarget(nullptr);

    // Execute a layout to determine initial window size.
    layout();
}

void Window_base::closingWindow() {
    Window* thisWindow = dynamic_cast<Window*>(this);
    assert(thisWindow);
    delegate->closingWindow(*thisWindow);

    auto lock = std::scoped_lock(guiMutex);
    state = State::NoWindow;
}

void Window_base::setDevice(Device *newDevice)
{
    auto lock = std::scoped_lock(guiMutex);

    if (device) {
        state = State::DeviceLost;
        teardown();
    }

    device = newDevice;
}



}
