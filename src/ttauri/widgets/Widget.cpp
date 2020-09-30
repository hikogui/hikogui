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

    _preferred_size = {
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

bool Widget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());
    return std::exchange(requestConstraint, false);
}

bool Widget::updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    need_layout |= std::exchange(requestLayout, false);
    if (need_layout) {
        // Used by draw().
        toWindowTransform = mat::T(window_rectangle.x(), window_rectangle.y(), z());

        // Used by handleMouseEvent()
        fromWindowTransform = ~toWindowTransform;
    }

    return need_layout;
}

DrawContext Widget::makeDrawContext(DrawContext context) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    context.clippingRectangle = clipping_rectangle();
    context.transform = toWindowTransform;

    // The default fill and border colors.
    context.color = theme->borderColor(nestingLevel());
    context.fillColor = theme->fillColor(nestingLevel());

    if (*enabled) {
        if (focus && window.active) {
            context.color = theme->accentColor;
        } else if (hover) {
            context.color = theme->borderColor(nestingLevel() + 1);
        }

        if (hover) {
            context.fillColor = theme->fillColor(nestingLevel() + 1);
        }

    } else {
        // Disabled, only the outline is shown.
        context.color = theme->borderColor(nestingLevel() - 1);
        context.fillColor = theme->fillColor(nestingLevel() - 1);
    }

    return context;
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
    ttlet lock = std::scoped_lock(mutex);

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

Widget const *Widget::nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    if (currentKeyboardWidget == nullptr && acceptsFocus()) {
        // The first widget that accepts focus.
        return this;

    } else {
        return nullptr;
    }
}

}
