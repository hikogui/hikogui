// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "label_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"

namespace tt {

label_widget::label_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    _icon_widget = std::make_unique<icon_widget>(window, this, label->icon);
    _icon_widget->alignment = alignment;
    _text_widget = std::make_unique<text_widget>(window, this, label->text);
    _text_widget->alignment = alignment;
    _text_widget->text_style = text_style;

    _label_callback = label.subscribe([this] {
        this->window.gui.run([this] {
            _icon_widget->icon = label->icon;
            _text_widget->text = label->text;
        });
    });
}

void label_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};
    _text_widget->constrain();
    _icon_widget->constrain();

    ttlet label_size = _text_widget->preferred_size();
    ttlet icon_size = _icon_widget->preferred_size();

    ttlet has_text = label_size.width() > 0.0f;
    ttlet has_icon = icon_size.width() > 0.0f;

    _inner_margin = (has_text and has_icon) ? theme().margin : 0.0f;

    if (has_icon) {
        // Override the natural icon size.
        if (*alignment == horizontal_alignment::center) {
            _icon_size = theme().large_icon_size;
        } else {
            _icon_size = std::ceil(theme().text_style(*text_style).scaled_size() * 1.4f);
        }
    } else {
        _icon_size = 0.0f;
    }

    auto size = label_size;
    if (has_icon) {
        if (*alignment != horizontal_alignment::center) {
            // If the icon is on the left or right, add the icon to the width.
            // Since the label is inline, we do not adjust the height of the label widget on the icon size.
            size.width() += _inner_margin + _icon_size;

        } else if (*alignment != vertical_alignment::middle) {
            // If the icon is above or below the text, add the icon height and the
            // minimum width is the maximum of the icon and text width.
            size.width() = std::max(size.width(), _icon_size);
            size.height() += _icon_size;

        } else {
            // The text is written across the icon. Take the maximum width and height
            // of both the icon and text.
            size.width() = std::max(size.width(), _icon_size);
            size.height() = std::max(size.height(), _icon_size);
        }
    }

    _minimum_size = size;
    _preferred_size = _minimum_size;
    _maximum_size = _preferred_size;
    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void label_widget::set_layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible) {
        if (_layout.store(context) >= layout_update::transform) {
            _text_rectangle = aarectangle{};
            if (*alignment == horizontal_alignment::left) {
                ttlet text_width = layout().width() - _icon_size - _inner_margin;
                _text_rectangle = {_icon_size + _inner_margin, 0.0f, text_width, layout().height()};

            } else if (*alignment == horizontal_alignment::right) {
                ttlet text_width = layout().width() - _icon_size - _inner_margin;
                _text_rectangle = {0.0f, 0.0f, text_width, layout().height()};

            } else if (*alignment == vertical_alignment::top) {
                ttlet text_height = layout().height() - _icon_size;
                _text_rectangle = {0.0f, 0.0f, layout().width(), text_height};

            } else if (*alignment == vertical_alignment::bottom) {
                ttlet text_height = layout().height() - _icon_size;
                _text_rectangle = {0.0f, _icon_size, layout().width(), text_height};

            } else {
                _text_rectangle = layout().rectangle();
            }

            auto icon_pos = point2{};
            switch (*alignment) {
            case alignment::top_left: icon_pos = {0.0f, layout().height() - _icon_size}; break;
            case alignment::top_right: icon_pos = {layout().width() - _icon_size, layout().height() - _icon_size}; break;
            case alignment::top_center:
                icon_pos = {(layout().width() - _icon_size) / 2.0f, layout().height() - _icon_size};
                break;
            case alignment::bottom_left: icon_pos = {0.0f, 0.0f}; break;
            case alignment::bottom_right: icon_pos = {layout().width() - _icon_size, 0.0f}; break;
            case alignment::bottom_center: icon_pos = {(layout().width() - _icon_size) / 2.0f, 0.0f}; break;
            case alignment::middle_left: icon_pos = {0.0f, (layout().height() - _icon_size) / 2.0f}; break;
            case alignment::middle_right: icon_pos = {layout().width() - _icon_size, (layout().height() - _icon_size)}; break;
            case alignment::middle_center:
                icon_pos = {(layout().width() - _icon_size) / 2.0f, (layout().height() - _icon_size)};
                break;
            default: tt_no_default();
            }
            _icon_rectangle = aarectangle{icon_pos, extent2{_icon_size, _icon_size}};
        }

        _icon_widget->set_layout(_icon_rectangle * context);
        _text_widget->set_layout(_text_rectangle * context);
    }
}

void label_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        _icon_widget->draw(context);
        _text_widget->draw(context);
    }
}

} // namespace tt
