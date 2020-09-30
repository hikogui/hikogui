// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_base.hpp"
#include "Window.hpp"
#include "GUIDevice.hpp"
#include "../widgets/WindowWidget.hpp"

namespace tt {

using namespace std;

Window_base::Window_base(WindowDelegate *delegate, Label &&title) :
    state(State::Initializing),
    delegate(delegate),
    title(std::move(title))
{
}

Window_base::~Window_base()
{
    // Destroy the top-level widget, before automatic destruction of the constraint solver
    // and other objects that the widgets require from the window during their destruction.
    widget.release();

    try {
        if (state != State::NoWindow) {
            LOG_FATAL("Window '{}' was not properly teardown before destruction.", title);
        }
        LOG_INFO("Window '{}' has been propertly destructed.", title);
    } catch (...) {
        abort();
    }
}

void Window_base::initialize()
{
    ttlet lock = std::scoped_lock(mutex);

    widget = std::make_unique<WindowWidget>(*static_cast<Window *>(this), delegate, title);

    // The delegate will populate the window with widgets.
    // This needs to be done first to figure out the initial size of the window.
    delegate->openingWindow(*static_cast<Window *>(this));

    // Execute a constraint check to determine initial window size.
    ttlet widget_lock = std::scoped_lock(widget->mutex);
    static_cast<void>(widget->updateConstraints());
    currentWindowExtent = widget->preferred_size().minimum();

    // Once the window is open, we should be a full constraint, layout and draw of the window.
    requestLayout = true;
    
    // Rest the keyboard target to not focus anything.
    updateToNextKeyboardTarget(nullptr);

    // Finished initializing the window.
    state = State::NoDevice;

    // Delegate has been called, layout of widgets has been calculated for the
    // minimum and maximum size of the window.
    createWindow(title.text(), currentWindowExtent);
}

bool Window_base::isClosed() {
    ttlet lock = std::scoped_lock(mutex);
    return state == State::NoWindow;
}

void Window_base::setDevice(GUIDevice *newDevice)
{
    ttlet lock = std::scoped_lock(mutex);

    if (device) {
        state = State::DeviceLost;
        teardown();
    }

    device = newDevice;
}

void Window_base::updateToNextKeyboardTarget(Widget *current_target_widget) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    auto *tmp = widget->nextKeyboardWidget(current_target_widget, false);
    if (tmp == current_target_widget) {
        // The currentTargetWidget was already the last (or only) widget.
        // cycle back to the first.
        tmp = widget->nextKeyboardWidget(nullptr, false);
    }

    updateKeyboardTarget(tmp);
}

void Window_base::updateToPrevKeyboardTarget(Widget *current_target_widget) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    auto *tmp = widget->nextKeyboardWidget(current_target_widget, true);
    if (tmp == current_target_widget) {
        // The currentTargetWidget was already the first (or only) widget.
        // cycle back to the last.
        tmp = widget->nextKeyboardWidget(nullptr, true);
    }

    updateKeyboardTarget(tmp);
}

[[nodiscard]] float Window_base::windowScale() const noexcept {
    ttlet lock = std::scoped_lock(mutex);

    return std::ceil(dpi / 100.0f);
}

void Window_base::windowChangedSize(ivec extent) {
    ttlet lock = std::scoped_lock(mutex);

    currentWindowExtent = extent;
    tt_assume(widget);

    ttlet widget_lock = std::scoped_lock(widget->mutex);
    widget->set_window_rectangle(aarect{vec{currentWindowExtent}});
    requestLayout = true;
}

void Window_base::updateMouseTarget(Widget const *newTargetWidget, vec position) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    if (newTargetWidget != mouseTargetWidget) {
        if (mouseTargetWidget != nullptr) {
            mouseTargetWidget->handleMouseEvent(MouseEvent::exited());
        }
        mouseTargetWidget = const_cast<Widget *>(newTargetWidget);
        if (mouseTargetWidget != nullptr) { 
            mouseTargetWidget->handleMouseEvent(MouseEvent::entered(position));
        }
    }
}

void Window_base::updateKeyboardTarget(Widget const *newTargetWidget) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    if (newTargetWidget) {
        ttlet widget_lock = std::scoped_lock(newTargetWidget->mutex);
        if (!newTargetWidget->acceptsFocus()) {
            newTargetWidget = nullptr;
        }
    }

    if (newTargetWidget != keyboardTargetWidget) {
        if (keyboardTargetWidget != nullptr) {
            ttlet widget_lock = std::scoped_lock(keyboardTargetWidget->mutex);
            keyboardTargetWidget->handleKeyboardEvent(KeyboardEvent::exited());
        }
        keyboardTargetWidget = const_cast<Widget *>(newTargetWidget);
        if (keyboardTargetWidget != nullptr) {
            ttlet widget_lock = std::scoped_lock(keyboardTargetWidget->mutex);
            keyboardTargetWidget->handleKeyboardEvent(KeyboardEvent::entered());
        }
    }
}

void Window_base::handleMouseEvent(MouseEvent event) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    switch (event.type) {
    case MouseEvent::Type::Exited: // Mouse left window.
        updateMouseTarget(nullptr);
        break;

    case MouseEvent::Type::ButtonDown:
    case MouseEvent::Type::Move: {
        ttlet hitbox = widget->hitBoxTest(event.position);
        updateMouseTarget(hitbox.widget, event.position);

        if (event.type == MouseEvent::Type::ButtonDown) {
            updateKeyboardTarget(hitbox.widget);
        }
        } break;
    default:;
    }

    // Send event to target-widget.
    if (mouseTargetWidget != nullptr) {
        mouseTargetWidget->handleMouseEvent(event);
    }
}

void Window_base::handleKeyboardEvent(KeyboardEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    if (keyboardTargetWidget != nullptr) {
        ttlet widget_lock = std::scoped_lock(keyboardTargetWidget->mutex);
        keyboardTargetWidget->handleKeyboardEvent(event);

    } else if (event.type == KeyboardEvent::Type::Key) {
        // If no widgets have been selected handle the keyboard-focus changes.
        for (ttlet command : event.getCommands()) {
            switch (command) {
            case command::gui_widget_next:
                updateToNextKeyboardTarget(nullptr);
                break;
            case command::gui_widget_prev:
                updateToPrevKeyboardTarget(nullptr);
                break;
            default:;
            }
        }
    }
}

void Window_base::handleKeyboardEvent(KeyboardState _state, KeyboardModifiers modifiers, KeyboardVirtualKey key) noexcept {
    return handleKeyboardEvent(KeyboardEvent(_state, modifiers, key));
}

void Window_base::handleKeyboardEvent(Grapheme grapheme, bool full) noexcept {
    return handleKeyboardEvent(KeyboardEvent(grapheme, full));
}

void Window_base::handleKeyboardEvent(char32_t c, bool full) noexcept {
    return handleKeyboardEvent(Grapheme(c), full);
}

}
