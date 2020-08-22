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

    delegate->openingWindow(*static_cast<Window *>(this));

    // Start of with a large window size, so that widgets have room to layout.
    // for the first time. Otherwise the minimum height is determined based on
    // the tiny width of 0 by 0 window.
    currentWindowExtent = ivec{500, 500};

    // Execute a layout to determine initial window size.
    ttlet widget_lock = std::scoped_lock(widget->mutex);
    if (widget->updateConstraints() >= WidgetUpdateResult::Children) {
        requestLayout = true;
        layoutWindow();
    }

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

rhea::constraint Window_base::addConstraint(rhea::constraint const& constraint) noexcept {
    auto lock = std::scoped_lock(widgetSolverMutex);

    widgetSolver.add_constraint(constraint);
    constraintsUpdated = true;
    return constraint;
}

rhea::constraint Window_base::addConstraint(
    rhea::linear_equation const& equation,
    rhea::strength const &strength,
    double weight
) noexcept {
    // Since equation has not been assigned to the solver, we do not need to lock
    // the widgetSolverMutex.

    return addConstraint(rhea::constraint(equation, strength, weight));
}

rhea::constraint Window_base::addConstraint(
    rhea::linear_inequality const& equation,
    rhea::strength const &strength,
    double weight
) noexcept {
    // Since equation has not been assigned to the solver, we do not need to lock
    // the widgetSolverMutex.

    return addConstraint(rhea::constraint(equation, strength, weight));
}

void Window_base::removeConstraint(rhea::constraint &constraint) noexcept {
    auto lock = std::scoped_lock(widgetSolverMutex);

    if (!constraint.is_nil()) {
        widgetSolver.remove_constraint(constraint);
        constraint = {};
    }
    constraintsUpdated = true;
}

void Window_base::replaceConstraint(
    rhea::constraint &oldConstraint,
    rhea::constraint const &newConstraint
) noexcept {
    stopConstraintSolver();

    {
        auto lock = std::scoped_lock(widgetSolverMutex);

        if (!oldConstraint.is_nil()) {
            widgetSolver.remove_constraint(oldConstraint);
            oldConstraint = {};
        }
        if (!newConstraint.is_nil()) {
            widgetSolver.add_constraint(newConstraint);
            oldConstraint = newConstraint;
        }
    }

    startConstraintSolver();
    constraintsUpdated = true;
}

void Window_base::replaceConstraint(
    rhea::constraint &oldConstraint,
    rhea::linear_equation const &newEquation,
    rhea::strength const &strength,
    double weight
) noexcept {
    // Since equation has not been assigned to the solver, we do not need to lock
    // the widgetSolverMutex.

    return replaceConstraint(oldConstraint, rhea::constraint(newEquation, strength, weight));
}

void Window_base::replaceConstraint(
    rhea::constraint &oldConstraint,
    rhea::linear_inequality const &newEquation,
    rhea::strength const &strength,
    double weight
) noexcept {
    // Since equation has not been assigned to the solver, we do not need to lock
    // the widgetSolverMutex.

    return replaceConstraint(oldConstraint, rhea::constraint(newEquation, strength, weight));
}

void Window_base::layoutWindow() noexcept {
    tt_assume(widget);
    ttlet lock = std::scoped_lock(mutex);
    ttlet widget_lock = std::scoped_lock(widget->mutex);

    if (state != Window_base::State::Initializing) {
        if (
            (currentWindowExtent.width() < widget->minimumExtent().width()) ||
            (currentWindowExtent.height() < widget->minimumExtent().height())
            ) {
            setWindowSize(widget->minimumExtent());
        }

        if (
            (currentWindowExtent.width() > widget->maximumExtent().width()) ||
            (currentWindowExtent.height() > widget->maximumExtent().height())
            ) {
            setWindowSize(widget->maximumExtent());
        }

    } else {
        currentWindowExtent = widget->preferredExtent();
    }

    // Set to actual window size.
    widget->setWindowExtent(currentWindowExtent);

    LOG_INFO("Window constraints '{}' minimumExtent={} preferredExtent={} maximumExtent={} currentExtent={}",
        title,
        widget->minimumExtent(),
        widget->preferredExtent(),
        widget->maximumExtent(),
        widget->windowExtent()
    );
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

void Window_base::updateToNextKeyboardTarget(Widget *currentTargetWidget) noexcept {
    ttlet lock = std::scoped_lock(mutex);
    ttlet widget_lock = std::scoped_lock(widget->mutex);

    auto tmp = widget->nextKeyboardWidget(currentTargetWidget, false);
    if (tmp == foundWidgetPtr) {
        tmp = nullptr;
    }

    updateKeyboardTarget(tmp);
}

void Window_base::updateToPrevKeyboardTarget(Widget *currentTargetWidget) noexcept {
    ttlet lock = std::scoped_lock(mutex);
    ttlet widget_lock = std::scoped_lock(widget->mutex);

    auto tmp = widget->nextKeyboardWidget(currentTargetWidget, true);
    if (tmp == foundWidgetPtr) {
        tmp = nullptr;
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
    widget->setWindowExtent(currentWindowExtent);
    requestLayout = true;
}

void Window_base::updateMouseTarget(Widget const *newTargetWidget, vec position) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    if (newTargetWidget != mouseTargetWidget) {
        if (mouseTargetWidget != nullptr) {
            ttlet widget_lock = std::scoped_lock(mouseTargetWidget->mutex);
            mouseTargetWidget->handleMouseEvent(MouseEvent::exited());
        }
        mouseTargetWidget = const_cast<Widget *>(newTargetWidget);
        if (mouseTargetWidget != nullptr) { 
            ttlet widget_lock = std::scoped_lock(mouseTargetWidget->mutex);
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
        ttlet hitbox = hitBoxTest(event.position);
        updateMouseTarget(hitbox.widget, event.position);

        if (event.type == MouseEvent::Type::ButtonDown) {
            updateKeyboardTarget(hitbox.widget);
        }
        } break;
    default:;
    }

    // Send event to target-widget.
    if (mouseTargetWidget != nullptr) {
        ttlet widget_lock = std::scoped_lock(mouseTargetWidget->mutex);

        ttlet windowOffset = mouseTargetWidget->offsetFromWindow;
        event.position -= windowOffset;
        event.downPosition -= windowOffset;
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

HitBox Window_base::hitBoxTest(vec position) const noexcept {
    ttlet lock = std::scoped_lock(mutex);
    ttlet widget_lock = std::scoped_lock(widget->mutex);
    return widget->hitBoxTest(position);
}



}
