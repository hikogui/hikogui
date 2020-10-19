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
    static_cast<void>(widget->update_constraints());
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

    auto *tmp = widget->next_keyboard_widget(current_target_widget, false);
    if (tmp == current_target_widget) {
        // The currentTargetWidget was already the last (or only) widget.
        // cycle back to the first.
        tmp = widget->next_keyboard_widget(nullptr, false);
    }

    updateKeyboardTarget(tmp);
}

void Window_base::updateToPrevKeyboardTarget(Widget *current_target_widget) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    auto *tmp = widget->next_keyboard_widget(current_target_widget, true);
    if (tmp == current_target_widget) {
        // The currentTargetWidget was already the first (or only) widget.
        // cycle back to the last.
        tmp = widget->next_keyboard_widget(nullptr, true);
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
    widget->set_layout_parameters(aarect{vec{currentWindowExtent}}, aarect{vec{currentWindowExtent}});
    requestLayout = true;
}

void Window_base::updateMouseTarget(Widget const *newTargetWidget, vec position) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    if (newTargetWidget != mouseTargetWidget) {
        if (mouseTargetWidget != nullptr) {
            mouseTargetWidget->handle_mouse_event(MouseEvent::exited());
        }
        mouseTargetWidget = const_cast<Widget *>(newTargetWidget);
        if (mouseTargetWidget != nullptr) { 
            mouseTargetWidget->handle_mouse_event(MouseEvent::entered(position));
        }
    }
}

void Window_base::updateKeyboardTarget(Widget const *newTargetWidget) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    if (newTargetWidget) {
        ttlet widget_lock = std::scoped_lock(newTargetWidget->mutex);
        if (!newTargetWidget->accepts_focus()) {
            newTargetWidget = nullptr;
        }
    }

    if (newTargetWidget != keyboardTargetWidget) {
        if (keyboardTargetWidget != nullptr) {
            keyboardTargetWidget->handle_keyboard_event(KeyboardEvent::exited());
        }
        keyboardTargetWidget = const_cast<Widget *>(newTargetWidget);
        if (keyboardTargetWidget != nullptr) {
            keyboardTargetWidget->handle_keyboard_event(KeyboardEvent::entered());
        }
    }
}

void Window_base::set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    tt_assume(widget);
    ttlet child_lock = std::scoped_lock(widget->mutex);
    return widget->set_resize_border_priority(left, right, bottom, top);
}

bool Window_base::handle_mouse_event(MouseEvent event) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    switch (event.type) {
    case MouseEvent::Type::Exited: // Mouse left window.
        updateMouseTarget(nullptr);
        break;

    case MouseEvent::Type::ButtonDown:
    case MouseEvent::Type::Move: {
        ttlet hitbox = widget->hitbox_test(event.position);
        updateMouseTarget(hitbox.widget, event.position);

        if (event.type == MouseEvent::Type::ButtonDown) {
            updateKeyboardTarget(hitbox.widget);
        }
        } break;
    default:;
    }

    auto target = mouseTargetWidget;
    while (target != nullptr) {
        if (target->handle_mouse_event(event)) {
            return true;
        }

        // Forward the mouse event to the parent of the target.
        target = target->parent;
    }
    
    return false;
}

bool Window_base::handle_keyboard_event(KeyboardEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    // Let the widget or its parent handle the keyboard event directly.
    auto target = keyboardTargetWidget;
    while (target != nullptr) {
        if (target->handle_keyboard_event(event)) {
            return true;
        }
        // Forward the keyboard event to the parent of the target.
        target = target->parent;
    }

    // If the keyboard event is not handled directly, convert the key event to a command.
    if (event.type == KeyboardEvent::Type::Key) {
        ttlet commands = event.getCommands();

        // Send the commands to the widget and its parents, until the command is handled.
        auto target = keyboardTargetWidget;
        while (target != nullptr) {
            for (auto command : commands) {
                // Send a command in priority order to the widget.
                if (target->handle_command(command)) {
                    return true;
                }
            }
            // Forward the keyboard event to the parent of the target.
            target = target->parent;
        }

        // If no widgets handle the commands, handle the keyboard focus change commands.
        for (ttlet command : commands) {
            switch (command) {
            case command::gui_widget_next:
                updateToNextKeyboardTarget(keyboardTargetWidget);
                return true;
            case command::gui_widget_prev:
                updateToPrevKeyboardTarget(keyboardTargetWidget);
                return true;
            default:;
            }
        }
    }

    return false;
}

bool Window_base::handle_keyboard_event(KeyboardState _state, KeyboardModifiers modifiers, KeyboardVirtualKey key) noexcept {
    return handle_keyboard_event(KeyboardEvent(_state, modifiers, key));
}

bool Window_base::handle_keyboard_event(Grapheme grapheme, bool full) noexcept {
    return handle_keyboard_event(KeyboardEvent(grapheme, full));
}

bool Window_base::handle_keyboard_event(char32_t c, bool full) noexcept {
    return handle_keyboard_event(Grapheme(c), full);
}

}
