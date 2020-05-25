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

bool Window_base::isClosed() {
    auto lock = std::scoped_lock(guiMutex);
    return state == State::NoWindow;
}

rhea::constraint Window_base::addConstraint(rhea::constraint const& constraint) noexcept {
    widgetSolver.add_constraint(constraint);
    constraintsUpdated = true;
    return constraint;
}

rhea::constraint Window_base::addConstraint(
    rhea::linear_equation const& equation,
    rhea::strength const &strength,
    double weight
) noexcept {
    return addConstraint(rhea::constraint(equation, strength, weight));
}

rhea::constraint Window_base::addConstraint(
    rhea::linear_inequality const& equation,
    rhea::strength const &strength,
    double weight
) noexcept {
    return addConstraint(rhea::constraint(equation, strength, weight));
}

void Window_base::removeConstraint(rhea::constraint const& constraint) noexcept {
    widgetSolver.remove_constraint(constraint);
    constraintsUpdated = true;
}

rhea::constraint Window_base::replaceConstraint(
    rhea::constraint const &oldConstraint,
    rhea::constraint const &newConstraint
) noexcept {
    widgetSolver.remove_constraint(oldConstraint);
    widgetSolver.add_constraint(newConstraint);
    constraintsUpdated = true;
    return newConstraint;
}

rhea::constraint Window_base::replaceConstraint(
    rhea::constraint const &oldConstraint,
    rhea::linear_equation const &newEquation,
    rhea::strength const &strength,
    double weight
) noexcept {
    return replaceConstraint(oldConstraint, rhea::constraint(newEquation, strength, weight));
}

rhea::constraint Window_base::replaceConstraint(
    rhea::constraint const &oldConstraint,
    rhea::linear_inequality const &newEquation,
    rhea::strength const &strength,
    double weight
) noexcept {
    return replaceConstraint(oldConstraint, rhea::constraint(newEquation, strength, weight));
}

void Window_base::layout(hires_utc_clock::time_point displayTimePoint)
{
    auto force = forceLayout.exchange(false);
    auto need = layoutChildren(displayTimePoint, force);
    if (force || need >= Widgets::WidgetNeed::Redraw) {
        forceRedraw = true;
    }
}

Widgets::WidgetNeed Window_base::layoutChildren(hires_utc_clock::time_point displayTimePoint, bool force) {
    constexpr int layout_retries = 10;

    auto total_need = Widgets::WidgetNeed::None;

    for (auto i = 0; i != layout_retries; ++i) {
        let child_need = widget->needs(displayTimePoint);
        total_need |= child_need;

        if (force || child_need >= Widgets::WidgetNeed::Layout) {
            widget->layout(displayTimePoint);
        }

        // Grandchildren need to be layed out when the child has changed.
        total_need |= widget->layoutChildren(displayTimePoint, force);

        // Layout may have changed the constraints, in that case recalculate them.
        if (constraintsUpdated) {
            constraintsUpdated = false;
            calculateMinimumAndMaximumWindowExtent();

        } else {
            return total_need;
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
    layout(cpu_utc_clock::now());
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

void Window_base::updateToNextKeyboardTarget(Widgets::Widget *currentTargetWidget) noexcept {
    Widgets::Widget *newTargetWidget =
        currentTargetWidget != nullptr ? currentTargetWidget->nextKeyboardWidget : firstKeyboardWidget;

    while (newTargetWidget != nullptr && !newTargetWidget->acceptsFocus()) {
        newTargetWidget = newTargetWidget->nextKeyboardWidget;
    }

    updateKeyboardTarget(newTargetWidget);
}

void Window_base::updateToPrevKeyboardTarget(Widgets::Widget *currentTargetWidget) noexcept {
    Widgets::Widget *newTargetWidget =
        currentTargetWidget != nullptr ? currentTargetWidget->prevKeyboardWidget : lastKeyboardWidget;

    while (newTargetWidget != nullptr && !newTargetWidget->acceptsFocus()) {
        newTargetWidget = newTargetWidget->prevKeyboardWidget;
    }

    updateKeyboardTarget(newTargetWidget);
}

[[nodiscard]] float Window_base::windowScale() const noexcept {
    return std::ceil(dpi / 100.0f);
}

void Window_base::windowChangedSize(ivec extent) {
    currentWindowExtent = extent;
    setWidgetToCurrentExtent();
    forceLayout = true;
}

void Window_base::updateMouseTarget(Widgets::Widget const *newTargetWidget) noexcept {
    if (newTargetWidget != mouseTargetWidget) {
        if (mouseTargetWidget != nullptr) {
            mouseTargetWidget->handleMouseEvent(MouseEvent::exited());
        }
        mouseTargetWidget = const_cast<Widgets::Widget *>(newTargetWidget);
        if (mouseTargetWidget != nullptr) {
            mouseTargetWidget->handleMouseEvent(MouseEvent::entered());
        }
    }
}

void Window_base::updateKeyboardTarget(Widgets::Widget const *newTargetWidget) noexcept {
    if (newTargetWidget == nullptr || !newTargetWidget->acceptsFocus()) {
        newTargetWidget = nullptr;
    }

    if (newTargetWidget != keyboardTargetWidget) {
        if (keyboardTargetWidget != nullptr) {
            keyboardTargetWidget->handleKeyboardEvent(KeyboardEvent::exited());
        }
        keyboardTargetWidget = const_cast<Widgets::Widget *>(newTargetWidget);
        if (keyboardTargetWidget != nullptr) {
            keyboardTargetWidget->handleKeyboardEvent(KeyboardEvent::entered());
        }
    }
}

void Window_base::handleMouseEvent(MouseEvent event) noexcept {
    switch (event.type) {
    case MouseEvent::Type::Exited: // Mouse left window.
        updateMouseTarget(nullptr);
        break;

    case MouseEvent::Type::ButtonDown:
    case MouseEvent::Type::Move: {
        let hitbox = hitBoxTest(event.position);
        updateMouseTarget(hitbox.widget);

        if (event.type == MouseEvent::Type::ButtonDown) {
            updateKeyboardTarget(hitbox.widget);
        }
        } break;
    default:;
    }

    // Send event to target-widget.
    if (mouseTargetWidget != nullptr) {
        event.position -= mouseTargetWidget->windowRectangle.offset();
        event.downPosition -= mouseTargetWidget->windowRectangle.offset();
        mouseTargetWidget->handleMouseEvent(event);
    }
}

void Window_base::handleKeyboardEvent(KeyboardEvent const &event) noexcept {
    if (keyboardTargetWidget != nullptr) {
        keyboardTargetWidget->handleKeyboardEvent(event);

    } else if (event.type == KeyboardEvent::Type::Key) {
        // If no widgets have been selected handle the keyboard-focus changes.
        for (let command : event.getCommands()) {
            if (command == "gui.widget.next"_ltag) {
                updateToNextKeyboardTarget(nullptr);
            } else if (command == "gui.widget.prev"_ltag) {
                updateToPrevKeyboardTarget(nullptr);
            }
        }
    }
}

void Window_base::handleKeyboardEvent(KeyboardState _state, KeyboardModifiers modifiers, KeyboardVirtualKey key) noexcept {
    return handleKeyboardEvent(KeyboardEvent(_state, modifiers, key));
}

void Window_base::handleKeyboardEvent(Text::Grapheme grapheme, bool full) noexcept {
    return handleKeyboardEvent(KeyboardEvent(grapheme, full));
}

void Window_base::handleKeyboardEvent(char32_t c, bool full) noexcept {
    return handleKeyboardEvent(Text::Grapheme(c), full);
}

HitBox Window_base::hitBoxTest(vec position) const noexcept {
    return widget->hitBoxTest(position);
}

void Window_base::setWidgetToCurrentExtent() {
    widgetSolver.suggest(widget->box.width, currentWindowExtent.width());
    widgetSolver.suggest(widget->box.height, currentWindowExtent.height());
}

void Window_base::calculateMinimumAndMaximumWindowExtent() {
    ttauri_assume(widget);

    // Test for minimum extent.
    widgetSolver.suggest(widget->box.width, 0);
    widgetSolver.suggest(widget->box.height, 0);
    minimumWindowExtent = widget->box.extent();

    // Test for maximum extent.
    widgetSolver.suggest(widget->box.width, std::numeric_limits<uint32_t>::max());
    widgetSolver.suggest(widget->box.height, std::numeric_limits<uint32_t>::max());
    maximumWindowExtent = widget->box.extent();

    if (
        (currentWindowExtent.x() < minimumWindowExtent.x()) ||
        (currentWindowExtent.y() < minimumWindowExtent.y())
        ) {
        setWindowSize(minimumWindowExtent);
    }

    if (
        (currentWindowExtent.x() > maximumWindowExtent.x()) ||
        (currentWindowExtent.y() > maximumWindowExtent.y())
        ) {
        setWindowSize(maximumWindowExtent);
    }

    // Set to actual window size.
    setWidgetToCurrentExtent();

    LOG_INFO("Window '{}' minimumExtent={} maximumExtent={} currentExtent={}",
        title,
        minimumWindowExtent,
        maximumWindowExtent,
        currentWindowExtent
    );

}

}
