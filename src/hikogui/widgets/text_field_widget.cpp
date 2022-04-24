// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_field_widget.hpp"
#include "../text/font_book.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"

namespace hi::inline v1 {

text_field_widget::text_field_widget(gui_window& window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept :
    super(window, parent), _delegate(std::move(delegate)), _text()
{
    if (auto d = _delegate.lock()) {
        _delegate_cbt = d->subscribe(*this, [&] {
            request_relayout();
        });
        d->init(*this);
    }

    _scroll_widget = std::make_unique<scroll_widget<axis::none, false>>(window, this);
    _text_widget = &_scroll_widget->make_widget<text_widget>(_text, hi::alignment::middle_flush());
    _text_widget->edit_mode = text_widget::edit_mode_type::line_editable;

    _error_label_widget =
        std::make_unique<label_widget>(window, this, _error_label, alignment::top_left(), theme_text_style::error);

    // clang-format off
    _continues_cbt = continues.subscribe([&](auto...){ request_reconstrain(); });
    _text_style_cbt = text_style.subscribe([&](auto...){ request_reconstrain(); });
    _text_cbt = _text.subscribe([&](auto...){ request_reconstrain(); });
    _error_label_cbt = _error_label.subscribe([&](auto...){ request_reconstrain(); });
    // clang-format on
}

text_field_widget::text_field_widget(gui_window& window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    text_field_widget(window, parent, weak_or_unique_ptr<delegate_type>{std::move(delegate)})
{
}

text_field_widget::~text_field_widget()
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

widget_constraints const& text_field_widget::set_constraints() noexcept
{
    if (*_text_widget->focus) {
        // Update the optional error value from the string conversion when the text-widget has keyboard focus.
        if (auto delegate = _delegate.lock()) {
            _error_label = delegate->validate(*this, to_string(*_text));
        } else {
            _error_label = {};
        }

    } else {
        // When field is not focused, simply follow the observed_value.
        revert(false);
    }

    _layout = {};

    auto size = extent2{};
    auto margins = hi::margins{theme().margin};

    hilet text_constraints = _scroll_widget->set_constraints();
    size.width() += 100.0f;
    size.height() += text_constraints.margins.top();
    size.height() += text_constraints.preferred.height();
    size.height() += text_constraints.margins.bottom();

    _error_label_widget->visible = not _error_label->empty();
    if (*_error_label_widget->visible) {
        hilet error_label_constraints = _error_label_widget->set_constraints();
        size.width() += error_label_constraints.preferred.width();
        size.height() += error_label_constraints.margins.top();
        size.height() += error_label_constraints.preferred.height();
        inplace_max(margins.left(), error_label_constraints.margins.left());
        inplace_max(margins.right(), error_label_constraints.margins.right());
        inplace_max(margins.bottom(), error_label_constraints.margins.bottom());
    }

    return _constraints = {size, size, size, theme().margin};
}

void text_field_widget::set_layout(widget_layout const& layout) noexcept
{
    if (compare_store(_layout, layout)) {
        if (*_error_label_widget->visible) {
            _error_label_rectangle =
                aarectangle{0.0f, 0.0f, layout.rectangle().width(), _error_label_widget->constraints().preferred.height()};

            _text_rectangle = aarectangle{point2{0.0f, _error_label_rectangle.height()}, get<3>(layout.rectangle())};
        } else {
            _text_rectangle = layout.rectangle();
        }
    }

    if (*_error_label_widget->visible) {
        _error_label_widget->set_layout(layout.transform(_error_label_rectangle));
    }
    _scroll_widget->set_layout(layout.transform(_text_rectangle));
}

void text_field_widget::draw(draw_context const& context) noexcept
{
    if (*visible and overlaps(context, layout())) {
        draw_background_box(context);

        _scroll_widget->draw(context);
        _error_label_widget->draw(context);
    }
}

bool text_field_widget::handle_event(gui_event const& event) noexcept
{
    switch (event.type()) {
    case gui_event_type::gui_cancel:
        if (*enabled) {
            revert(true);
            return true;
        }
        break;

    case gui_event_type::gui_activate:
        if (*enabled) {
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
    hi_axiom(is_gui_thread());

    if (*visible and *enabled) {
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
    if (*visible and *enabled) {
        return _scroll_widget->accepts_keyboard_focus(group);
    } else {
        return false;
    }
}

[[nodiscard]] color text_field_widget::focus_color() const noexcept
{
    if (*enabled) {
        if (not _error_label->empty()) {
            return theme().text_style(theme_text_style::error).color;
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
    if (auto delegate = _delegate.lock()) {
        _text = to_gstring(delegate->text(*this), U' ');
    } else {
        _text = {};
    }
    _error_label = {};
}

void text_field_widget::commit(bool force) noexcept
{
    hi_axiom(is_gui_thread());
    if (*continues or force) {
        auto text = to_string(*_text);

        if (auto delegate = _delegate.lock()) {
            if (delegate->validate(*this, text).empty()) {
                // text is valid.
                delegate->set_text(*this, text);
            }

            // After commit get the canonical text to display from the delegate.
            _text = to_gstring(delegate->text(*this), U' ');
        } else {
            _text = {};
        }
        _error_label = {};
    }
}

void text_field_widget::draw_background_box(draw_context const& context) const noexcept
{
    hilet corner_radii = hi::corner_radii{0.0f, 0.0f, theme().rounding_radius, theme().rounding_radius};
    context.draw_box(_layout, _text_rectangle, background_color(), corner_radii);

    hilet line = line_segment(get<0>(_text_rectangle), get<1>(_text_rectangle));
    context.draw_line(_layout, translate3{0.0f, 0.5f, 0.1f} * line, theme().border_width, focus_color());
}

} // namespace hi::inline v1
