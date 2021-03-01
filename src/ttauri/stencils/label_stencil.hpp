// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stencil.hpp"
#include "text_stencil.hpp"
#include "image_stencil.hpp"
#include "../label.hpp"
#include "../GUI/theme.hpp"
#include "../GUI/draw_context.hpp"
#include <string_view>

namespace tt {

class label_stencil : public stencil {
public:
    label_stencil(alignment alignment, label label, text_style style) noexcept :
        stencil(alignment),
        _style(style),
        _icon_size(_alignment == horizontal_alignment::center ? theme::global->large_icon_size : theme::global->icon_size)
    {
        if (label.has_icon()) {
            _icon_stencil = stencil::make_unique(alignment::middle_center, label.icon());
        }
        if (label.has_text()) {
            _text_stencil = stencil::make_unique(alignment, label.text(), style);
        }
    }

    [[nodiscard]] extent2 preferred_extent() noexcept override
    {
        if (!_text_stencil) {
            // There is only an icon available.
            return {_icon_size, _icon_size};
        }

        if (!_icon_stencil && !_show_icon) {
            // There is no image, just use the text label.
            return _text_stencil->preferred_extent();
        }

        // clang-format off
        // When center aligned, do not include the icon width. So that the icon may go beyond the margins.
        ttlet width =
            _alignment == horizontal_alignment::center ? _text_stencil->preferred_extent().width() :
            _icon_size + theme::global->margin + _text_stencil->preferred_extent().width();

        // When middle aligned, do not include the icon height. So that the icon may go beyond the margins.
        ttlet height =
            _alignment == vertical_alignment::middle ? _text_stencil->preferred_extent().height() :
            _icon_size + _text_stencil->preferred_extent().height();
        // clang-format on

        return {width, height};
    }

    /** Whether the text in the label will align to an optional icon in the label.
     * Make space for, and optionally display, an icon in front
     * of the text. This option should be used when any of the labels in a menu
     * has an icon.
     *
     * This should not be used when a menu is displayed in the same direction as
     * the icon label. For example a left or right aligned menu item in a row menu;
     * such as the tool-bar.
     *
     * @retval true The text of the label will be aligned after an optional icon of the label.
     * @retval false The text of the label will be not be aligned to an optional icon of the label.
     */
    [[nodiscard]] bool show_icon() const noexcept
    {
        return _show_icon;
    }

    /** Set the `show_icon()` flag.
     */
    void set_show_icon(bool flag) noexcept
    {
        if (_show_icon != flag) {
            _show_icon = flag;
            _size_is_modified = true;
            _position_is_modified = true;
        }
    }

    void set_layout_parameters(
        aarect const &rectangle,
        float base_line_position = std::numeric_limits<float>::infinity()) noexcept override
    {
        stencil::set_layout_parameters(rectangle, base_line_position);

        // clang-format off
        ttlet icon_x =
            _alignment == horizontal_alignment::left ? _rectangle.left() :
            _alignment == horizontal_alignment::center ? _rectangle.center() - _icon_size * 0.5f :
            _rectangle.right() - _icon_size;

        ttlet icon_y = 
            _alignment == vertical_alignment::bottom ? _rectangle.bottom() :
            _alignment == vertical_alignment::middle ? _rectangle.middle() - _icon_size * 0.5f :
            _rectangle.top() - _icon_size;
        // clang-format on


        if (_icon_stencil) {
            ttlet icon_rectangle = aarect{icon_x, icon_y, _icon_size, _icon_size};
            _icon_stencil->set_layout_parameters(icon_rectangle);
        }

        if (_text_stencil) {
            // clang-format off
            ttlet text_width =
                _alignment == horizontal_alignment::center ? _rectangle.width() :
                _icon_stencil || _show_icon ? _rectangle.width() - theme::global->margin - _icon_size :
                _rectangle.width();

            ttlet text_height =
                _alignment == vertical_alignment::middle ? _rectangle.height() :
                _icon_stencil || _show_icon ? _rectangle.height() - _icon_size :
                _rectangle.height();

            ttlet text_x =
                _alignment == horizontal_alignment::center ? _rectangle.center() - text_width * 0.5f :
                _alignment == horizontal_alignment::left ? _rectangle.right() - text_width :
                _rectangle.left();

            ttlet text_y =
                _alignment == vertical_alignment::middle ? _rectangle.middle() - text_height * 0.5f :
                _alignment == vertical_alignment::bottom ? _rectangle.top() - text_height :
                _rectangle.bottom();
            // clang-format on

            ttlet text_rectangle = aarect{text_x, text_y, text_width, text_height};
            if (_alignment == horizontal_alignment::center) {
                _text_stencil->set_layout_parameters(text_rectangle);
            } else {
                _text_stencil->set_layout_parameters(text_rectangle, _base_line_position);
            }
        }
    }

    void draw(draw_context context, tt::color color, matrix3 transform = geo::identity{}) noexcept override
    {
        if (_text_stencil) {
            _text_stencil->draw(context, color, transform);
        }
        if (_icon_stencil) {
            _icon_stencil->draw(context, color, transform);
        }
    }

private:
    bool _show_icon = false;
    text_style _style;
    float _icon_size;
    std::unique_ptr<image_stencil> _icon_stencil;
    std::unique_ptr<text_stencil> _text_stencil;
};

} // namespace tt
