// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_field_widget.hpp"
#include "../text/font_book.hpp"

namespace hi::inline v1 {

text_field_widget::text_field_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept :
    super(parent), delegate(std::move(delegate)), _text()
{
    hi_assert_not_null(this->delegate);
    _delegate_cbt = this->delegate->subscribe([&] {
        ++global_counter<"text_field_widget:delegate:layout">;
        process_event({gui_event_type::window_relayout});
    });
    this->delegate->init(*this);

    _scroll_widget = std::make_shared<scroll_widget<axis::none>>(this);
    _text_widget = &_scroll_widget->make_widget<text_widget>(_text, alignment, text_style);
    _text_widget->mode = widget_mode::partial;

    _error_label_widget = std::make_shared<label_widget>(this, _error_label, alignment::top_left(), semantic_text_style::error);

    _continues_cbt = continues.subscribe([&](auto...) {
        ++global_counter<"text_field_widget:continues:constrain">;
        process_event({gui_event_type::window_reconstrain});
    });
    _text_style_cbt = text_style.subscribe([&](auto...) {
        ++global_counter<"text_field_widget:text_style:constrain">;
        process_event({gui_event_type::window_reconstrain});
    });
    _text_cbt = _text.subscribe([&](auto...) {
        ++global_counter<"text_field_widget:text:constrain">;
        process_event({gui_event_type::window_reconstrain});
    });
    _error_label_cbt = _error_label.subscribe([&](auto const& new_value) {
        ++global_counter<"text_field_widget:error_label:constrain">;
        process_event({gui_event_type::window_reconstrain});
    });
}

text_field_widget::~text_field_widget()
{
    hi_assert_not_null(delegate);
    delegate->deinit(*this);
}

[[nodiscard]] generator<widget *> text_field_widget::children() const noexcept
{
    co_yield _scroll_widget.get();
}

[[nodiscard]] box_constraints text_field_widget::get_constraints(get_constraints_context const& context) noexcept
{
    hi_assert_not_null(delegate);

    if (*_text_widget->focus) {
        // Update the optional error value from the string conversion when the text-widget has keyboard focus.
        _error_label = delegate->validate(*this, to_string(*_text));

    } else {
        // When field is not focused, simply follow the observed_value.
        revert(false);
    }

    _layout = {};

    auto margins = hi::margins{context.theme->margin};

    hilet scroll_width = 100.0f;
    _scroll_constraints = _scroll_widget->get_constraints(context);

    hilet box_size = extent2{
        narrow_cast<float>(_scroll_constraints.margin_left) + scroll_width + narrow_cast<float>(_scroll_constraints.margin_right),
        narrow_cast<float>(_scroll_constraints.margin_top) + narrow_cast<float>(_scroll_constraints.preferred_height) +
                narrow_cast<float>(_scroll_constraints.margin_bottom)};

    auto size = box_size;
    if (_error_label->empty()) {
        _error_label_widget->mode = widget_mode::invisible;
        _error_label_constraints = {};

    } else {
        _error_label_widget->mode = widget_mode::display;
        _error_label_constraints = _error_label_widget->get_constraints(context);
        inplace_max(size.width(), narrow_cast<float>(_error_label_constraints.preferred_width));
        size.height() += narrow_cast<float>(_error_label_constraints.margin_top) +
            narrow_cast<float>(_error_label_constraints.preferred_height);
        inplace_max(margins.left(), narrow_cast<float>(_error_label_constraints.margin_left));
        inplace_max(margins.right(), narrow_cast<float>(_error_label_constraints.margin_right));
        inplace_max(margins.bottom(), narrow_cast<float>(_error_label_constraints.margin_bottom));
    }

    // The alignment of a text-field is not based on the text-widget due to the intermediate scroll widget.
    hilet resolved_alignment = resolve_mirror(*alignment, context.left_to_right());

    return {size, size, size, resolved_alignment, context.theme->margin};
}

void text_field_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet scroll_size = extent2{
            narrow_cast<float>(context.width()),
            narrow_cast<float>(_scroll_constraints.margin_top) + narrow_cast<float>(_scroll_constraints.preferred_height) +
                narrow_cast<float>(_scroll_constraints.margin_bottom)};

        hilet scroll_rectangle =
            aarectangle{point2{0.0f, narrow_cast<float>(context.height()) - scroll_size.height()}, scroll_size};
        _scroll_shape = box_shape{_scroll_constraints, scroll_rectangle, context.theme->baseline_adjustment};

        if (*_error_label_widget->mode > widget_mode::invisible) {
            hilet error_label_rectangle =
                aarectangle{0.0f, 0.0f, context.rectangle().width(), narrow_cast<float>(_error_label_constraints.preferred_height)};
            _error_label_shape = box_shape{_error_label_constraints, error_label_rectangle, context.theme->baseline_adjustment};
        }
    }

    if (*_error_label_widget->mode > widget_mode::invisible) {
        _error_label_widget->set_layout(context.transform(_error_label_shape));
    }
    _scroll_widget->set_layout(context.transform(_scroll_shape));
}

void text_field_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        draw_background_box(context);

        _scroll_widget->draw(context);
        _error_label_widget->draw(context);
    }
}

bool text_field_widget::handle_event(gui_event const& event) noexcept
{
    switch (event.type()) {
    case gui_event_type::gui_cancel:
        if (*mode >= widget_mode::partial) {
            revert(true);
            return true;
        }
        break;

    case gui_event_type::gui_activate:
        if (*mode >= widget_mode::partial) {
            commit(true);
            return super::handle_event(event);
        }
        break;

    default:;
    }

    return super::handle_event(event);
}

hitbox text_field_widget::hitbox_test(point3 position) const noexcept
{
    if (*mode >= widget_mode::partial) {
        auto r = hitbox{};
        r = _scroll_widget->hitbox_test_from_parent(position, r);
        r = _error_label_widget->hitbox_test_from_parent(position, r);
        return r;
    } else {
        return hitbox{};
    }
}

[[nodiscard]] bool text_field_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    if (*mode >= widget_mode::partial) {
        return _scroll_widget->accepts_keyboard_focus(group);
    } else {
        return false;
    }
}

[[nodiscard]] color text_field_widget::focus_color() const noexcept
{
    if (*mode >= widget_mode::partial) {
        if (not _error_label->empty()) {
            return _layout.theme->text_style(semantic_text_style::error)->color;
        } else if (*_text_widget->focus) {
            return _layout.theme->color(semantic_color::accent);
        } else if (*hover) {
            return _layout.theme->color(semantic_color::border, semantic_layer + 1);
        } else {
            return _layout.theme->color(semantic_color::border, semantic_layer);
        }

    } else {
        return _layout.theme->color(semantic_color::border, semantic_layer - 1);
    }
}

void text_field_widget::revert(bool force) noexcept
{
    hi_assert_not_null(delegate);
    _text = to_gstring(delegate->text(*this), U' ');
    _error_label = label{};
}

void text_field_widget::commit(bool force) noexcept
{
    hi_axiom(loop::main().on_thread());
    hi_assert_not_null(delegate);

    if (*continues or force) {
        auto text = to_string(*_text);

        if (delegate->validate(*this, text).empty()) {
            // text is valid.
            delegate->set_text(*this, text);
        }

        // After commit get the canonical text to display from the delegate.
        _text = to_gstring(delegate->text(*this), U' ');
        _error_label = label{};
    }
}

void text_field_widget::draw_background_box(draw_context const& context) const noexcept
{
    hilet scroll_rectangle = _scroll_shape.rectangle();

    hilet corner_radii = hi::corner_radii{0.0f, 0.0f, layout().theme->rounding_radius, layout().theme->rounding_radius};
    context.draw_box(layout(), scroll_rectangle, background_color(), corner_radii);

    hilet line = line_segment(get<0>(scroll_rectangle), get<1>(scroll_rectangle));
    context.draw_line(layout(), translate3{0.0f, 0.5f, 0.1f} * line, layout().theme->border_width, focus_color());
}

} // namespace hi::inline v1
