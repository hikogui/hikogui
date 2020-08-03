// Copyright 2019 Pokitec
// All rights reserved.

#include "Widget.hpp"
#include "../GUI/utils.hpp"

namespace tt {

Widget::Widget(Window &_window, Widget *_parent) noexcept :
    window(_window),
    parent(_parent),
    elevation(_parent ? _parent->elevation + 1.0f : 0.0f),
    enabled(true)
    
{
    [[maybe_unused]] ttlet enabled_cbid = enabled.add_callback([this](auto...){
        window.requestRedraw = true;
    });
}

Widget::~Widget()
{
    window.removeConstraint(minimumWidthConstraint);
    window.removeConstraint(minimumHeightConstraint);
    window.removeConstraint(maximumWidthConstraint);
    window.removeConstraint(maximumHeightConstraint);
    window.removeConstraint(baseConstraint);
}

GUIDevice *Widget::device() const noexcept
{
    auto device = window.device;
    tt_assert(device);
    return device;
}

[[nodiscard]] float Widget::baseHeight() const noexcept {
    ttlet lock = std::scoped_lock(window.widgetSolverMutex);
    return numeric_cast<float>(base.value() - bottom.value());
}

rhea::constraint Widget::placeBelow(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->top + margin == rhs.bottom);
}

rhea::constraint Widget::placeAbove(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->bottom == rhs.top + margin);
}

rhea::constraint Widget::placeLeftOf(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->right + margin == rhs.left);
}

rhea::constraint Widget::placeRightOf(Widget const &rhs, float margin) const noexcept {
    return window.addConstraint(this->left == rhs.right + margin);
}

rhea::constraint Widget::placeAtTop(float margin) const noexcept {
    return window.addConstraint(this->top + margin == parent->top);
}

rhea::constraint Widget::placeAtBottom(float margin) const noexcept {
    return window.addConstraint(this->bottom - margin == parent->bottom);
}

rhea::constraint Widget::placeLeft(float margin) const noexcept {
    return window.addConstraint(this->left - margin == parent->left);
}

rhea::constraint Widget::placeRight(float margin) const noexcept {
    return window.addConstraint(this->right + margin == parent->right);
}

bool Widget::updateConstraints() noexcept
{
    return requestConstraint.exchange(false, std::memory_order::memory_order_relaxed);
}

bool Widget::updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept
{
    auto needLayout = forceLayout;

    needLayout |= requestLayout.exchange(false, std::memory_order::memory_order_relaxed);

    auto newExtent = round(vec{width.value(), height.value()});
    needLayout |= newExtent != extent();
    setExtent(newExtent);

    auto newOffsetFromWindow = round(vec{left.value(), bottom.value()});
    needLayout |= newOffsetFromWindow != offsetFromWindow();
    setOffsetFromWindow(newOffsetFromWindow);
    
    if (needLayout) {
        auto lock = std::scoped_lock(mutex);

        setOffsetFromParent(
            parent ?
                offsetFromWindow() - parent->offsetFromWindow():
                offsetFromWindow()
        );
        
        fromWindowTransform = mat::T(-offsetFromWindow().x(), -offsetFromWindow().y(), -z());
        toWindowTransform = mat::T(offsetFromWindow().x(), offsetFromWindow().y(), z());
    }

    return needLayout;
}

void Widget::handleCommand(command command) noexcept {
    auto lock = std::scoped_lock(mutex);

    switch (command) {
    case command::gui_widget_next:
        window.updateToNextKeyboardTarget(this);
        break;
    case command::gui_widget_prev:
        window.updateToPrevKeyboardTarget(this);
        break;
    default:;
    }
}

void Widget::handleMouseEvent(MouseEvent const &event) noexcept {
    auto lock = std::scoped_lock(mutex);

    if (event.type == MouseEvent::Type::Entered) {
        hover = true;
        window.requestRedraw = true;
    } else if (event.type == MouseEvent::Type::Exited) {
        hover = false;
        window.requestRedraw = true;
    }
}

void Widget::handleKeyboardEvent(KeyboardEvent const &event) noexcept {
    auto lock = std::scoped_lock(mutex);

    switch (event.type) {
    case KeyboardEvent::Type::Entered:
        focus = true;
        window.requestRedraw = true;
        break;

    case KeyboardEvent::Type::Exited:
        focus = false;
        window.requestRedraw = true;
        break;

    case KeyboardEvent::Type::Key:
        for (ttlet command : event.getCommands()) {
            handleCommand(command);
        }
        break;

    default:;
    }
}

Widget *Widget::nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
{
    if (currentKeyboardWidget == nullptr && acceptsFocus()) {
        // The first widget that accepts focus.
        return const_cast<Widget *>(this);

    } else {
        return nullptr;
    }
}

}
