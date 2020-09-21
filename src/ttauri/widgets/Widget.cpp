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

    _size = {
        vec{0.0f, 0.0f},
        vec{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}
    };
}

Widget::~Widget()
{
}

GUIDevice *Widget::device() const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto device = window.device;
    tt_assert(device);
    return device;
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
    
    if (needLayout) {
        toWindowTransform = mat::T(_window_rectangle.x(), _window_rectangle.y(), z());
        fromWindowTransform = ~toWindowTransform;

        if (parent) {
            offsetFromParent = _window_rectangle.p0() - parent->window_rectangle().p0();
        } else {
            offsetFromParent = _window_rectangle.p0();
        }
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
