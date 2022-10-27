// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_field_widget.hpp"
#include "../text/font_book.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"

namespace hi::inline v1 {

text_field_widget::text_field_widget(gui_window& window, widget *parent, std::shared_ptr<delegate_type> delegate) noexcept :
    super(window, parent), delegate(std::move(delegate)), _text()
{
    hi_assert_not_null(this->delegate);
    _delegate_cbt = this->delegate->subscribe([&] {
        request_relayout();
    });
    this->delegate->init(*this);

    _scroll_widget = std::make_unique<scroll_widget<axis::none, false>>(window, this);
    _text_widget = &_scroll_widget->make_widget<text_widget>(_text, alignment, text_style);
    _text_widget->mode = widget_mode::partial;

    _error_label_widget =
        std::make_unique<label_widget>(window, this, _error_label, alignment::top_left(), semantic_text_style::error);

    _continues_cbt = continues.subscribe([&](auto...) {
        hi_request_reconstrain("text_field_widget::_continues_cbt()");
    });
    _text_style_cbt = text_style.subscribe([&](auto...) {
        hi_request_reconstrain("text_field_widget::_text_style_cbt()");
    });
    _text_cbt = _text.subscribe([&](auto...) {
        hi_request_reconstrain("text_field_widget::_text_cbt()");
    });
    _error_label_cbt = _error_label.subscribe([&](auto const& new_value) {
        hi_request_reconstrain("text_field_widget::_error_label_cbt(\"{}\")", new_value);
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

widget_constraints const& text_field_widget::set_constraints() noexcept
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

    auto margins = hi::margins{theme().margin};

    hilet scroll_width = 100.0f;
    _text_constraints = _scroll_widget->set_constraints();

    hilet box_size = extent2{
        _text_constraints.margins.left() + scroll_width + _text_constraints.margins.right(),
        _text_constraints.margins.top() + _text_constraints.preferred.height() + _text_constraints.margins.bottom()};

    auto size = box_size;
    if (_error_label->empty()) {
        _error_label_widget->mode = widget_mode::invisible;
        _error_label_constraints = {};

    } else {
        _error_label_widget->mode = widget_mode::display;
        _error_label_constraints = _error_label_widget->set_constraints();
        inplace_max(size.width(), _error_label_constraints.preferred.width());
        size.height() += _error_label_constraints.margins.top() + _error_label_constraints.preferred.height();
        inplace_max(margins.left(), _error_label_constraints.margins.left());
        inplace_max(margins.right(), _error_label_constraints.margins.right());
        inplace_max(margins.bottom(), _error_label_constraints.margins.bottom());
    }

    return _constraints = {
               size,
               size,
               size,
               theme().margin,
               widget_baseline{0.5f, vertical_alignment::top, theme().cap_height, box_size.height()}};
}

void text_field_widget::set_layout(widget_layout const& layout) noexcept
{
    if (compare_store(_layout, layout)) {
        hilet box_size = extent2{
            layout.width(),
            _text_constraints.margins.top() + _text_constraints.preferred.height() + _text_constraints.margins.bottom()};

        _box_rectangle = aarectangle{point2{0.0f, layout.height() - box_size.height()}, box_size};
        _text_rectangle = _box_rectangle - theme().border_width;

        if (*_error_label_widget->mode > widget_mode::invisible) {
            _error_label_rectangle =
                aarectangle{0.0f, 0.0f, layout.rectangle().width(), _error_label_constraints.preferred.height()};
        }
    }

    if (*_error_label_widget->mode > widget_mode::invisible) {
        _error_label_widget->set_layout(layout.transform(_error_label_rectangle, _error_label_constraints.baseline));
    }
    _scroll_widget->set_layout(layout.transform(_text_rectangle));
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
    hi_axiom(is_gui_thread());
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
    hilet corner_radii = hi::corner_radii{0.0f, 0.0f, theme().rounding_radius, theme().rounding_radius};
    context.draw_box(_layout, _box_rectangle, background_color(), corner_radii);

    hilet line = line_segment(get<0>(_box_rectangle), get<1>(_box_rectangle));
    context.draw_line(_layout, translate3{0.0f, 0.5f, 0.1f} * line, theme().border_width, focus_color());
}

} // namespace hi::inline v1
