// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_field_widget.hpp"
#include "../font/module.hpp"

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

    _scroll_widget = std::make_unique<scroll_widget<axis::none>>(this);
    _text_widget = &_scroll_widget->make_widget<text_widget>(_text, alignment, text_style);
    _text_widget->mode = widget_mode::partial;

    _error_label_widget = std::make_unique<label_widget>(this, _error_label, alignment::top_left(), semantic_text_style::error);

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

[[nodiscard]] generator<widget const&> text_field_widget::children(bool include_invisible) const noexcept
{
    co_yield *_scroll_widget;
}

[[nodiscard]] box_constraints text_field_widget::update_constraints() noexcept
{
    hi_assert_not_null(delegate);
    hi_assert_not_null(_error_label_widget);
    hi_assert_not_null(_scroll_widget);

    if (*_text_widget->focus) {
        // Update the optional error value from the string conversion when the text-widget has keyboard focus.
        _error_label = delegate->validate(*this, to_string(*_text));

    } else {
        // When field is not focused, simply follow the observed_value.
        revert(false);
    }

    _layout = {};
    _scroll_constraints =_scroll_widget->update_constraints();

    hilet scroll_width = 100;
    hilet box_size = extent2i{
        _scroll_constraints.margins.left() + scroll_width + _scroll_constraints.margins.right(),
        _scroll_constraints.margins.top() + _scroll_constraints.preferred.height() + _scroll_constraints.margins.bottom()};

    auto size = box_size;
    auto margins = theme().margin();
    if (_error_label->empty()) {
        _error_label_widget->mode = widget_mode::invisible;
        _error_label_constraints = _error_label_widget->update_constraints();

    } else {
        _error_label_widget->mode = widget_mode::display;
        _error_label_constraints = _error_label_widget->update_constraints();
        inplace_max(size.width(), _error_label_constraints.preferred.width());
        size.height() += _error_label_constraints.margins.top() + _error_label_constraints.preferred.height();
        inplace_max(margins.left(), _error_label_constraints.margins.left());
        inplace_max(margins.right(), _error_label_constraints.margins.right());
        inplace_max(margins.bottom(), _error_label_constraints.margins.bottom());
    }

    // The alignment of a text-field is not based on the text-widget due to the intermediate scroll widget.
    hilet resolved_alignment = resolve_mirror(*alignment, os_settings::left_to_right());

    return {size, size, size, resolved_alignment, margins};
}

void text_field_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet scroll_size = extent2i{
            context.width(),
            _scroll_constraints.margins.top() + _scroll_constraints.preferred.height() + _scroll_constraints.margins.bottom()};

        hilet scroll_rectangle = aarectanglei{point2i{0, context.height() - scroll_size.height()}, scroll_size};
        _scroll_shape = box_shape{_scroll_constraints, scroll_rectangle, theme().baseline_adjustment()};

        if (*_error_label_widget->mode > widget_mode::invisible) {
            hilet error_label_rectangle =
                aarectanglei{0, 0, context.rectangle().width(), _error_label_constraints.preferred.height()};
            _error_label_shape = box_shape{_error_label_constraints, error_label_rectangle, theme().baseline_adjustment()};
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

hitbox text_field_widget::hitbox_test(point2i position) const noexcept
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
            return theme().text_style(semantic_text_style::error)->color;
        } else if (*_text_widget->focus) {
            return theme().color(semantic_color::accent);
        } else if (*hover) {
            return theme().color(semantic_color::border, semantic_layer + 1);
        } else {
            return theme().color(semantic_color::border, semantic_layer);
        }

    } else {
        return theme().color(semantic_color::border, semantic_layer - 1);
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
    hilet outline = narrow_cast<aarectangle>(_scroll_shape.rectangle);

    hilet corner_radii = hi::corner_radii(0.0f, 0.0f, theme().rounding_radius<float>(), theme().rounding_radius<float>());
    context.draw_box(layout(), outline, background_color(), corner_radii);

    hilet line = line_segment(get<0>(outline), get<1>(outline));
    context.draw_line(layout(), translate3{0.0f, 0.5f, 0.1f} * line, theme().border_width(), focus_color());
}

} // namespace hi::inline v1
