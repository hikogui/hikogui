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
    tt_assume(mutex.is_locked_by_current_thread());

    auto device = window.device;
    tt_assert(device);
    return device;
}

[[nodiscard]] float Widget::baseHeight() const noexcept {
    return numeric_cast<float>(base.value() - bottom.value());
}

WidgetUpdateResult Widget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    return requestConstraint.exchange(false, std::memory_order::memory_order_relaxed) ?
        WidgetUpdateResult::Self :
        WidgetUpdateResult::Nothing;
}

WidgetUpdateResult Widget::updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto needLayout = forceLayout;

    needLayout |= requestLayout.exchange(false, std::memory_order::memory_order_relaxed);

    auto newExtent = round(vec{width.value(), height.value()});
    needLayout |= newExtent != extent;
    extent = newExtent;

    auto newOffsetFromWindow = round(vec{left.value(), bottom.value()});
    needLayout |= newOffsetFromWindow != offsetFromWindow;
    offsetFromWindow = newOffsetFromWindow;
    
    if (needLayout) {
        offsetFromParent = parent ?
            offsetFromWindow - parent->offsetFromWindow:
            offsetFromWindow;
        
        toWindowTransform = mat::T(offsetFromWindow.x(), offsetFromWindow.y(), z());
        fromWindowTransform = ~toWindowTransform;
    }

    return needLayout ? WidgetUpdateResult::Self : WidgetUpdateResult::Nothing;
}

void Widget::handleCommand(command command) noexcept {
    tt_assume(mutex.is_locked_by_current_thread());

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
    tt_assume(mutex.is_locked_by_current_thread());

    if (event.type == MouseEvent::Type::Entered) {
        hover = true;
        window.requestRedraw = true;
    } else if (event.type == MouseEvent::Type::Exited) {
        hover = false;
        window.requestRedraw = true;
    }
}

void Widget::handleKeyboardEvent(KeyboardEvent const &event) noexcept {
    tt_assume(mutex.is_locked_by_current_thread());

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
    tt_assume(mutex.is_locked_by_current_thread());

    if (currentKeyboardWidget == nullptr && acceptsFocus()) {
        // The first widget that accepts focus.
        return const_cast<Widget *>(this);

    } else {
        return nullptr;
    }
}

}
