// Copyright 2019 Pokitec
// All rights reserved.

#include "ButtonWidget.hpp"
#include "../GUI/utils.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

using namespace std::literals;

ButtonWidget::ButtonWidget(Window &window, Widget *parent) noexcept : Widget(window, parent)
{
    [[maybe_unused]] ttlet label_cbid = label.add_callback([this](auto...) {
        request_reconstrain = true;
    });
}

ButtonWidget::~ButtonWidget() {}

[[nodiscard]] bool ButtonWidget::update_constraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (Widget::update_constraints()) {
        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
        p_preferred_size = interval_vec2::make_minimum(labelCell->preferredExtent() + Theme::margin2Dx2);
        return true;
    } else {
        return false;
    }
}

void ButtonWidget::draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    context.cornerShapes = vec{Theme::roundingRadius};
    if (value) {
        context.fillColor = theme->accentColor;
    }

    // Move the border of the button in the middle of a pixel.
    context.drawBoxIncludeBorder(rectangle());

    if (*enabled) {
        context.color = theme->foregroundColor;
    }
    context.transform = mat::T{0.0f, 0.0f, 0.1f} * context.transform;
    labelCell->draw(context, rectangle(), Alignment::MiddleCenter, base_line(), true);

    Widget::draw(std::move(context), display_time_point);
}

bool ButtonWidget::handle_command(command command) noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    auto handled = Widget::handle_command(command);

    if (*enabled) {
        if (command == command::gui_activate) {
            handled = true;
            if (compare_then_assign(value, !value)) {
                window.requestRedraw = true;
            }
        }
    }

    return handled;
}

bool ButtonWidget::handle_mouse_event(MouseEvent const &event) noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    auto handled = Widget::handle_mouse_event(event);

    if (event.cause.leftButton) {
        handled = true;
        if (*enabled) {
            if (compare_then_assign(pressed, static_cast<bool>(event.down.leftButton))) {
                window.requestRedraw = true;
            }

            if (event.type == MouseEvent::Type::ButtonUp && p_window_rectangle.contains(event.position)) {
                handle_command(command::gui_activate);
            }
        }
    }
    return handled;
}

HitBox ButtonWidget::hitbox_test(vec window_position) const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    if (p_window_clipping_rectangle.contains(window_position)) {
        return HitBox{this, p_draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

} // namespace tt
