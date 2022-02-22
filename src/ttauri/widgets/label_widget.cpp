// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "label_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"

namespace tt::inline v1 {

label_widget::label_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    _icon_widget = std::make_unique<icon_widget>(window, this, label->icon);
    _icon_widget->alignment = alignment;
    _text_widget = std::make_unique<text_widget>(window, this, to_gstring(label->text()));
    _text_widget->alignment = alignment;
    _text_widget->text_style = text_style;
    _text_widget->edit_mode = edit_mode;

    _text_style_callback = text_style.subscribe([this] {
        switch (*text_style) {
        case theme_text_style::label: _icon_widget->color = theme_color::foreground; break;
        case theme_text_style::small_label: _icon_widget->color = theme_color::foreground; break;
        case theme_text_style::warning: _icon_widget->color = theme_color::orange; break;
        case theme_text_style::error: _icon_widget->color = theme_color::red; break;
        case theme_text_style::help: _icon_widget->color = theme_color::indigo; break;
        case theme_text_style::placeholder: _icon_widget->color = theme_color::gray; break;
        case theme_text_style::link: _icon_widget->color = theme_color::blue; break;
        default: _icon_widget->color = theme_color::foreground;
        }
    });

    _label_callback = label.subscribe([this] {
        _icon_widget->icon = label->icon;
        _text_widget->text = to_gstring(label->text());
    });
}

widget_constraints const &label_widget::set_constraints() noexcept
{
    _layout = {};

    // Translate the text of the label during reconstrain as this is triggered when the system language changes.
    _text_widget->text = to_gstring(label->text());
    ttlet &text_constraints = _text_widget->set_constraints();
    ttlet &icon_constraints = _icon_widget->set_constraints();

    ttlet label_size = text_constraints.preferred;
    ttlet icon_size = icon_constraints.preferred;

    ttlet has_text = label_size.width() > 0.0f;
    ttlet has_icon = icon_size.width() > 0.0f;

    _inner_margin = (has_text and has_icon) ? theme().margin : 0.0f;

    _icon_size = [&] {
        if (has_icon) {
            // Override the natural icon size.
            if (alignment == horizontal_alignment::center or alignment == horizontal_alignment::justified) {
                return theme().large_icon_size;
            } else {
                return std::ceil(theme().text_style(*text_style).size * theme().scale);
            }
        } else {
            return 0.0f;
        }
    }();

    ttlet size = [&] {
        if (has_icon) {
            if (alignment != horizontal_alignment::center and alignment != horizontal_alignment::justified) {
                // If the icon is on the left or right, add the icon to the width.
                // Since the label is inline, we do not adjust the height of the label widget on the icon size.
                return extent2{label_size.width() + _inner_margin + _icon_size, label_size.height()};

            } else if (alignment != vertical_alignment::middle) {
                // If the icon is above or below the text, add the icon height and the
                // minimum width is the maximum of the icon and text width.
                return extent2{std::max(label_size.width(), _icon_size), label_size.height() + _inner_margin + _icon_size};

            } else {
                // The text is written across the icon. Take the maximum width and height
                // of both the icon and text.
                return extent2{std::max(label_size.width(), _icon_size), std::max(label_size.height(), _icon_size)};
            }
        } else {
            return label_size;
        }
    }();

    return _constraints = {size, size, size, theme().margin};
}

void label_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _text_rectangle = aarectangle{};
        if (alignment == horizontal_alignment::left) {
            ttlet text_width = layout.width() - _icon_size - _inner_margin;
            _text_rectangle = {_icon_size + _inner_margin, 0.0f, text_width, layout.height()};

        } else if (alignment == horizontal_alignment::right) {
            ttlet text_width = layout.width() - _icon_size - _inner_margin;
            _text_rectangle = {0.0f, 0.0f, text_width, layout.height()};

        } else if (alignment == vertical_alignment::top) {
            ttlet text_height = layout.height() - _icon_size - _inner_margin;
            _text_rectangle = {0.0f, 0.0f, layout.width(), text_height};

        } else if (alignment == vertical_alignment::bottom) {
            ttlet text_height = layout.height() - _icon_size - _inner_margin;
            _text_rectangle = {0.0f, _icon_size + _inner_margin, layout.width(), text_height};

        } else {
            _text_rectangle = layout.rectangle();
        }

        ttlet icon_pos = [&] {
            if (alignment == tt::alignment::top_left()) {
                return point2{0.0f, layout.height() - _icon_size};
            } else if (alignment == tt::alignment::top_right()) {
                return point2{layout.width() - _icon_size, layout.height() - _icon_size};
            } else if (alignment == vertical_alignment::top) {
                return point2{(layout.width() - _icon_size) / 2.0f, layout.height() - _icon_size};
            } else if (alignment == tt::alignment::bottom_left()) {
                return point2{0.0f, 0.0f};
            } else if (alignment == tt::alignment::bottom_right()) {
                return point2{layout.width() - _icon_size, 0.0f};
            } else if (alignment == vertical_alignment::bottom) {
                return point2{(layout.width() - _icon_size) / 2.0f, 0.0f};
            } else if (alignment == tt::alignment::middle_left()) {
                return point2{0.0f, (layout.height() - _icon_size) / 2.0f};
            } else if (alignment == tt::alignment::middle_right()) {
                return point2{layout.width() - _icon_size, (layout.height() - _icon_size)};
            } else if (alignment == vertical_alignment::middle) {
                return point2{(layout.width() - _icon_size) / 2.0f, (layout.height() - _icon_size)};
            } else {
                tt_no_default();
            }
        }();
        _icon_rectangle = aarectangle{icon_pos, extent2{_icon_size, _icon_size}};
    }

    // Elevate the child widget by 0.0f since the label widget does not draw itself.
    _icon_widget->set_layout(layout.transform(_icon_rectangle, 0.0f));
    _text_widget->set_layout(layout.transform(_text_rectangle, 0.0f));
}

void label_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        _icon_widget->draw(context);
        _text_widget->draw(context);
    }
}

[[nodiscard]] hitbox label_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        return _text_widget->hitbox_test_from_parent(position);
    } else {
        return {};
    }
}

} // namespace tt::inline v1
