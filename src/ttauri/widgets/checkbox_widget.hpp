// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt {

class checkbox_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    checkbox_widget(gui_window &window, widget *parent, unique_or_borrow_ptr<delegate_type> delegate) noexcept :
        super(window, parent, std::move(delegate))
    {
        label_alignment = alignment::top_left;
    }

    template<typename Value, typename... Args>
    requires(not std::is_convertible_v<Value, unique_or_borrow_ptr<delegate_type>>)
        checkbox_widget(gui_window &window, widget *parent, Value &&value, Args &&...args) noexcept :
        checkbox_widget(
            window,
            parent,
            make_unique_default_button_delegate<button_type::toggle>(std::forward<Value>(value), std::forward<Args>(args)...))
    {
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

        need_layout |= _request_relayout.exchange(false);
        if (need_layout) {
            _button_rectangle = align(rectangle(), _button_size, alignment::top_left);

            _label_rectangle = aarectangle{_button_rectangle.right() + theme::global().margin, 0.0f, width(), height()};

            _check_glyph = to_font_glyph_ids(elusive_icon::Ok);
            ttlet check_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_check_glyph);
            _check_glyph_rectangle =
                align(_button_rectangle, scale(check_glyph_bb, theme::global().icon_size), alignment::middle_center);

            _minus_glyph = to_font_glyph_ids(elusive_icon::Minus);
            ttlet minus_glyph_bb = pipeline_SDF::device_shared::getBoundingBox(_minus_glyph);
            _minus_glyph_rectangle =
                align(_button_rectangle, scale(minus_glyph_bb, theme::global().icon_size), alignment::middle_center);
        }
        super::update_layout(displayTimePoint, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(is_gui_thread());

        if (overlaps(context, _clipping_rectangle)) {
            draw_check_box(context);
            draw_check_mark(context);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    extent2 _button_size;
    aarectangle _button_rectangle;
    font_glyph_ids _check_glyph;
    aarectangle _check_glyph_rectangle;
    font_glyph_ids _minus_glyph;
    aarectangle _minus_glyph_rectangle;

    void draw_check_box(draw_context const &context) noexcept
    {
        tt_axiom(is_gui_thread());
        context.draw_box_with_border_inside(_button_rectangle, background_color(), focus_color());
    }

    void draw_check_mark(draw_context context) noexcept
    {
        tt_axiom(is_gui_thread());

        auto state_ = state();

        // Checkmark or tristate.
        if (state_ == tt::button_state::on) {
            context.draw_glyph(_check_glyph, translate_z(0.1f) * _check_glyph_rectangle, accent_color());

        } else if (state_ == tt::button_state::off) {
            ;

        } else {
            context.draw_glyph(_minus_glyph, translate_z(0.1f) * _minus_glyph_rectangle, accent_color());
        }
    }
};

} // namespace tt
