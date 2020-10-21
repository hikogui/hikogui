// Copyright 2019 Pokitec
// All rights reserved.

#include "Widget.hpp"
#include "../GUI/utils.hpp"

namespace tt {

Widget::Widget(Window &_window, Widget *_parent) noexcept :
    enabled(true),
    window(_window),
    parent(_parent),
    p_draw_layer(0.0f),
    p_logical_layer(0),
    p_semantic_layer(0)
{
    if (_parent) {
        ttlet lock = std::scoped_lock(_parent->mutex);
        p_draw_layer = _parent->draw_layer() + 1.0f;
        p_logical_layer = _parent->logical_layer() + 1;
        p_semantic_layer = _parent->semantic_layer() + 1;
    }

    _enabled_callback = scoped_callback(enabled, [this](auto...){
        window.requestRedraw = true;
    });

    p_preferred_size = {
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

bool Widget::update_constraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());
    return std::exchange(request_reconstrain, false);
}

bool Widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    need_layout |= std::exchange(request_relayout, false);
    if (need_layout) {
        // Used by draw().
        to_window_transform = mat::T(p_window_rectangle.x(), p_window_rectangle.y(), p_draw_layer);

        // Used by handle_mouse_event()
        from_window_transform = ~to_window_transform;
    }

    return need_layout;
}

DrawContext Widget::make_draw_context(DrawContext context) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    context.clippingRectangle = p_window_clipping_rectangle;
    context.transform = to_window_transform;

    // The default fill and border colors.
    context.color = theme->borderColor(p_semantic_layer);
    context.fillColor = theme->fillColor(p_semantic_layer);

    if (*enabled) {
        if (focus && window.active) {
            context.color = theme->accentColor;
        } else if (hover) {
            context.color = theme->borderColor(p_semantic_layer + 1);
        }

        if (hover) {
            context.fillColor = theme->fillColor(p_semantic_layer + 1);
        }

    } else {
        // Disabled, only the outline is shown.
        context.color = theme->borderColor(p_semantic_layer - 1);
        context.fillColor = theme->fillColor(p_semantic_layer - 1);
    }

    return context;
}

bool Widget::handle_command(command command) noexcept
{
    return false;
}

bool Widget::handle_mouse_event(MouseEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(mutex);
    auto handled = false;

    if (event.type == MouseEvent::Type::Entered) {
        handled = true;
        hover = true;
        window.requestRedraw = true;

    } else if (event.type == MouseEvent::Type::Exited) {
        handled = true;
        hover = false;
        window.requestRedraw = true;
    }
    return handled;
}

bool Widget::handle_keyboard_event(KeyboardEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(mutex);
    auto handled = false;

    switch (event.type) {
    case KeyboardEvent::Type::Entered:
        handled = true;
        focus = true;
        window.requestRedraw = true;
        break;

    case KeyboardEvent::Type::Exited:
        handled = true;
        focus = false;
        window.requestRedraw = true;
        break;

    default:;
    }

    return handled;
}

Widget const *Widget::next_keyboard_widget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    if (currentKeyboardWidget == nullptr && accepts_focus()) {
        // The first widget that accepts focus.
        return this;

    } else {
        return nullptr;
    }
}

}
