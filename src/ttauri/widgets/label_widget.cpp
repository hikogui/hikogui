// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "label_widget.hpp"

namespace tt {

label_widget::~label_widget() {}

label_widget::label_widget(
    gui_window &window,
    std::shared_ptr<widget> parent,
    std::shared_ptr<label_delegate> delegate,
    alignment alignment,
    text_style text_style) noexcept :
    super(window, std::move(parent), std::move(delegate)), _alignment(alignment), _text_style(text_style)
{
}

void label_widget::init() noexcept
{
    _icon_widget = super::make_widget<icon_widget>(delegate_ptr<label_delegate>(), _alignment);
    _text_widget = super::make_widget<text_widget>(delegate_ptr<label_delegate>(), _alignment, _text_style);
}

[[nodiscard]] tt::label label_widget::label() const noexcept
{
    return delegate<label_delegate>().label(*this);
}

void label_widget::set_label(observable<tt::label> label) noexcept
{
    return delegate<label_delegate>().set_label(*this, std::move(label));
}

void label_widget::set_label(l10n label) noexcept
{
    return delegate<label_delegate>().set_label(*this, tt::label{label});
}

[[nodiscard]] bool
label_widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        ttlet label_size = _text_widget->preferred_size();
        ttlet icon_size = _icon_widget->preferred_size();

        ttlet has_text = label_size.width() > 0.0f;
        ttlet has_icon = icon_size.width() > 0.0f;

        _inner_margin = (has_text and has_icon) ? theme::global->margin : 0.0f;

        if (has_icon) {
            // Override the natural icon size.
            if (_alignment == horizontal_alignment::center) {
                _icon_size = theme::global->large_icon_size;
            } else {
                _icon_size = theme::global->icon_size;
            }
        } else {
            _icon_size = 0.0f;
        }

        auto size = label_size;
        if (has_icon) {
            if (_alignment != horizontal_alignment::center) {
                // If the icon is on the left or right, add the icon to the width and
                // the minimum height is the maximum of the icon and text height.
                size.width() += _inner_margin + _icon_size;
                size.height() = std::max(size.height(), _icon_size);

            } else if (_alignment != vertical_alignment::middle) {
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
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void label_widget::update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_layout |= std::exchange(this->_request_relayout, false);
    if (need_layout) {
        auto text_rect = aarectangle{};
        if (_alignment == horizontal_alignment::left) {
            ttlet text_width = width() - _icon_size - _inner_margin;
            text_rect = {_icon_size + _inner_margin, 0.0f, text_width, height()};

        } else if (_alignment == horizontal_alignment::right) {
            ttlet text_width = width() - _icon_size - _inner_margin;
            text_rect = {0.0f, 0.0f, text_width, height()};

        } else if (_alignment == vertical_alignment::top) {
            ttlet text_height = height() - _icon_size;
            text_rect = {0.0f, 0.0f, width(), text_height};

        } else if (_alignment == vertical_alignment::bottom) {
            ttlet text_height = height() - _icon_size;
            text_rect = {0.0f, _icon_size, width(), text_height};

        } else {
            text_rect = rectangle();
        }

        auto icon_pos = point2{};
        switch (_alignment) {
        case alignment::top_left: icon_pos = {0.0f, height() - _icon_size}; break;
        case alignment::top_right: icon_pos = {width() - _icon_size, height() - _icon_size}; break;
        case alignment::top_center: icon_pos = {(width() - _icon_size) / 2.0f, height() - _icon_size}; break;
        case alignment::bottom_left: icon_pos = {0.0f, 0.0f}; break;
        case alignment::bottom_right: icon_pos = {width() - _icon_size, 0.0f}; break;
        case alignment::bottom_center: icon_pos = {(width() - _icon_size) / 2.0f, 0.0f}; break;
        case alignment::middle_left: icon_pos = {0.0f, (height() - _icon_size) / 2.0f}; break;
        case alignment::middle_right: icon_pos = {width() - _icon_size, (height() - _icon_size)}; break;
        case alignment::middle_center: icon_pos = {(width() - _icon_size) / 2.0f, (height() - _icon_size)}; break;
        default: tt_no_default();
        }
        ttlet icon_rect = aarectangle{icon_pos, extent2{_icon_size, _icon_size}};

        _icon_widget->set_layout_parameters_from_parent(icon_rect);
        _text_widget->set_layout_parameters_from_parent(text_rect);
    }
    super::update_layout(displayTimePoint, need_layout);
}

}
