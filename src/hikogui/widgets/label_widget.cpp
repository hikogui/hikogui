// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "label_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"

namespace hi::inline v1 {

label_widget::label_widget(gui_window& window, widget *parent) noexcept : super(window, parent)
{
    mode = widget_mode::select;

    _icon_widget = std::make_unique<icon_widget>(window, this, label.get<"icon">());
    _icon_widget->alignment = alignment;
    _text_widget = std::make_unique<text_widget>(window, this, label.get<"text">());
    _text_widget->alignment = alignment;
    _text_widget->text_style = text_style;
    _text_widget->mode = mode;

    _text_style_cbt = text_style.subscribe(
        [this](auto...) {
            switch (*text_style) {
            case semantic_text_style::label:
                _icon_widget->color = color::foreground();
                break;
            case semantic_text_style::small_label:
                _icon_widget->color = color::foreground();
                break;
            case semantic_text_style::warning:
                _icon_widget->color = color::orange();
                break;
            case semantic_text_style::error:
                _icon_widget->color = color::red();
                break;
            case semantic_text_style::help:
                _icon_widget->color = color::indigo();
                break;
            case semantic_text_style::placeholder:
                _icon_widget->color = color::gray();
                break;
            case semantic_text_style::link:
                _icon_widget->color = color::blue();
                break;
            default:
                _icon_widget->color = color::foreground();
            }
        },
        callback_flags::main);
}

widget_constraints const& label_widget::set_constraints() noexcept
{
    _layout = {};

    // Translate the text of the label during reconstrain as this is triggered when the system language changes.
    _text_constraints = _text_widget->set_constraints();
    _icon_constraints = _icon_widget->set_constraints();

    hilet label_size = _text_constraints.preferred;
    hilet icon_size = _icon_constraints.preferred;

    hilet has_text = label_size.width() > 0.0f;
    hilet has_icon = icon_size.width() > 0.0f;

    _inner_margin = (has_text and has_icon) ? theme().margin : 0.0f;

    _icon_size = [&] {
        if (has_icon) {
            // Override the natural icon size.
            if (*alignment == horizontal_alignment::center or *alignment == horizontal_alignment::justified) {
                return theme().large_icon_size;
            } else {
                return std::ceil(theme().text_style(*text_style)->size * theme().scale);
            }
        } else {
            return 0.0f;
        }
    }();

    hilet size = [&] {
        if (has_icon) {
            if (*alignment != horizontal_alignment::center and *alignment != horizontal_alignment::justified) {
                // If the icon is on the left or right, add the icon to the width.
                // Since the label is inline, we do not adjust the height of the label widget on the icon size.
                return extent2{label_size.width() + _inner_margin + _icon_size, label_size.height()};

            } else if (*alignment != vertical_alignment::middle) {
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

    if ((*alignment == horizontal_alignment::center or *alignment == horizontal_alignment::justified) and
        *alignment != vertical_alignment::middle) {
        // When the icon and text are above one another, the label needs to define its own base-line.
        return _constraints = {size, size, size, theme().margin};
    } else {
        return _constraints = {size, size, size, theme().margin, _text_constraints.baseline};
    }
}

void label_widget::set_layout(widget_layout const& layout) noexcept
{
    if (compare_store(_layout, layout)) {
        hilet alignment_ = layout.left_to_right() ? *alignment : mirror(*alignment);

        _text_rectangle = aarectangle{};
        if (alignment_ == horizontal_alignment::left) {
            hilet text_width = layout.width() - _icon_size - _inner_margin;
            _text_rectangle = {_icon_size + _inner_margin, 0.0f, text_width, layout.height()};

        } else if (alignment_ == horizontal_alignment::right) {
            hilet text_width = layout.width() - _icon_size - _inner_margin;
            _text_rectangle = {0.0f, 0.0f, text_width, layout.height()};

        } else if (alignment_ == vertical_alignment::top) {
            hilet text_height = layout.height() - _icon_size - _inner_margin;
            _text_rectangle = {0.0f, 0.0f, layout.width(), text_height};

        } else if (alignment_ == vertical_alignment::bottom) {
            hilet text_height = layout.height() - _icon_size - _inner_margin;
            _text_rectangle = {0.0f, _icon_size + _inner_margin, layout.width(), text_height};

        } else {
            _text_rectangle = layout.rectangle();
        }

        hilet icon_pos = [&] {
            if (alignment_ == hi::alignment::top_left()) {
                return point2{0.0f, layout.height() - _icon_size};
            } else if (alignment_ == hi::alignment::top_right()) {
                return point2{layout.width() - _icon_size, layout.height() - _icon_size};
            } else if (alignment_ == vertical_alignment::top) {
                return point2{(layout.width() - _icon_size) / 2.0f, layout.height() - _icon_size};
            } else if (alignment_ == hi::alignment::bottom_left()) {
                return point2{0.0f, 0.0f};
            } else if (alignment_ == hi::alignment::bottom_right()) {
                return point2{layout.width() - _icon_size, 0.0f};
            } else if (alignment_ == vertical_alignment::bottom) {
                return point2{(layout.width() - _icon_size) / 2.0f, 0.0f};
            } else if (alignment_ == hi::alignment::middle_left()) {
                return point2{0.0f, (layout.height() - _icon_size) / 2.0f};
            } else if (alignment_ == hi::alignment::middle_right()) {
                return point2{layout.width() - _icon_size, (layout.height() - _icon_size) / 2.0f};
            } else if (alignment_ == vertical_alignment::middle) {
                return point2{(layout.width() - _icon_size) / 2.0f, (layout.height() - _icon_size) / 2.0f};
            } else {
                hi_no_default();
            }
        }();
        _icon_rectangle = aarectangle{icon_pos, extent2{_icon_size, _icon_size}};
    }

    // Elevate the child widget by 0.0f since the label widget does not draw itself.
    if ((*alignment == horizontal_alignment::center or *alignment == horizontal_alignment::justified) and
        *alignment != vertical_alignment::middle) {
        // When the icon and text are above one another, the label needs to define its own base-line.
        _icon_widget->set_layout(layout.transform(_icon_rectangle, 0.0f, _icon_constraints.baseline));
        _text_widget->set_layout(layout.transform(_text_rectangle, 0.0f, _text_constraints.baseline));
    } else {
        _icon_widget->set_layout(layout.transform(_icon_rectangle, 0.0f));
        _text_widget->set_layout(layout.transform(_text_rectangle, 0.0f));
    }
}

void label_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        _icon_widget->draw(context);
        _text_widget->draw(context);
    }
}

[[nodiscard]] hitbox label_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(is_gui_thread());

    if (*mode > widget_mode::invisible) {
        return _text_widget->hitbox_test_from_parent(position);
    } else {
        return {};
    }
}

} // namespace hi::inline v1
