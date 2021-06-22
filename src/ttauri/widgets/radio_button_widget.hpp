// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "value_button_delegate.hpp"

namespace tt {

class radio_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    radio_button_widget(gui_window &window, std::shared_ptr<widget> parent, std::shared_ptr<delegate_type> delegate) noexcept :
        super(window, std::move(parent), std::move(delegate))
    {
        label_alignment = alignment::top_left;
    }

    template<typename Label, typename Value, typename OnValue>
    radio_button_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        Label &&label,
        Value &&value,
        OnValue &&on_value) noexcept :
        radio_button_widget(
            window,
            std::move(parent),
            make_value_button_delegate<button_type::radio>(std::forward<Value>(value), std::forward<OnValue>(on_value)))
    {
        set_label(std::forward<Label>(label));
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            // Make room for button and margin.
            _button_size = {theme::global().size, theme::global().size};
            ttlet extra_size = extent2{theme::global().margin + _button_size.width(), 0.0f};
            _minimum_size += extra_size;
            _preferred_size += extra_size;
            _maximum_size += extra_size;

            _minimum_size = max(_minimum_size, _button_size);
            _preferred_size = max(_minimum_size, _button_size);
            _maximum_size = max(_minimum_size, _button_size);

            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _button_rectangle = align(rectangle(), _button_size, alignment::top_left);

            _label_rectangle = aarectangle{_button_rectangle.right() + theme::global().margin, 0.0f, width(), height()};

            _pip_rectangle = align(
                _button_rectangle,
                extent2{theme::global().icon_size, theme::global().icon_size},
                alignment::middle_center);
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (overlaps(context, _clipping_rectangle)) {
            draw_radio_button(context);
            draw_radio_pip(context, display_time_point);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    static constexpr hires_utc_clock::duration _animation_duration = 150ms;

    extent2 _button_size;
    aarectangle _button_rectangle;
    animator<float> _animated_value = _animation_duration;
    aarectangle _pip_rectangle;

    void draw_radio_button(draw_context const &context) noexcept
    {
        tt_axiom(is_gui_thread());

        context.draw_box_with_border_inside(
            _button_rectangle, background_color(), focus_color(), corner_shapes{_button_rectangle.height() * 0.5f});
    }

    void draw_radio_pip(draw_context const &context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_axiom(is_gui_thread());

        _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, display_time_point);
        if (_animated_value.is_animating()) {
            request_redraw();
        }

        // draw pip
        auto float_value = _animated_value.current_value();
        if (float_value > 0.0) {
            ttlet scaled_pip_rectangle = scale(_pip_rectangle, float_value);
            context.draw_box(scaled_pip_rectangle, accent_color(), corner_shapes{scaled_pip_rectangle.height() * 0.5f});
        }
    }
};

} // namespace tt
