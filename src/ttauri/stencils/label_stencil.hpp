// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil.hpp"
#include "text_stencil.hpp"
#include "image_stencil.hpp"
#include "../label.hpp"
#include <string_view>

namespace tt {

class label_stencil : public stencil {
public:
    label_stencil(Alignment alignment, label label, TextStyle style) noexcept :
        stencil(alignment),
        _style(style),
        _icon_size(_alignment == HorizontalAlignment::Center ? _style.size * 3.0f : _style.size * 2.0f)
    {
        if (label.has_icon()) {
            _icon_stencil = stencil::make_unique(Alignment::MiddleCenter, label.icon());
        }
        if (label.has_text()) {
            _text_stencil = stencil::make_unique(alignment, label.text(), style);
        }
    }

    [[nodiscard]] vec preferred_extent() noexcept override
    {
        if (!_text_stencil) {
            // There is only an icon available.
            return {_icon_size, _icon_size};
        }

        if (!_icon_stencil && !_always_show_icon) {
            // There is no image, just use the text label.
            return _text_stencil->preferred_extent();
        }

        // clang-format off
        // When center aligned, do not include the icon width. So that the icon may go beyond the margins.
        ttlet width =
            _alignment == HorizontalAlignment::Center ? _text_stencil->preferred_extent().width() :
            _icon_size + Theme::margin + _text_stencil->preferred_extent().width();

        // When middle aligned, do not include the icon height. So that the icon may go beyond the margins.
        ttlet height =
            _alignment == VerticalAlignment::Middle ? _text_stencil->preferred_extent().height() :
            _icon_size + _text_stencil->preferred_extent().height();
        // clang-format on

        return {width, height};
    }

    void set_layout_parameters(
        aarect const &rectangle,
        float base_line_position = std::numeric_limits<float>::infinity()) noexcept override
    {
        stencil::set_layout_parameters(rectangle, base_line_position);

        // clang-format off
        ttlet icon_x =
            _alignment == HorizontalAlignment::Left ? _rectangle.left() :
            _alignment == HorizontalAlignment::Center ? _rectangle.center() - _icon_size * 0.5f :
            _rectangle.right() - _icon_size;

        ttlet icon_y = 
            _alignment == VerticalAlignment::Bottom ? _rectangle.bottom() :
            _alignment == VerticalAlignment::Middle ? _rectangle.middle() - _icon_size * 0.5f :
            _rectangle.top() - _icon_size;
        // clang-format on


        if (_icon_stencil) {
            ttlet icon_rectangle = aarect{icon_x, icon_y, _icon_size, _icon_size};
            _icon_stencil->set_layout_parameters(icon_rectangle);
        }

        if (_text_stencil) {
            // clang-format off
            ttlet text_width =
                _alignment == HorizontalAlignment::Center ? _rectangle.width() :
                _icon_stencil || _always_show_icon ? _rectangle.width() - Theme::margin - _icon_size :
                _rectangle.width();

            ttlet text_height =
                _alignment == VerticalAlignment::Middle ? _rectangle.height() :
                _icon_stencil || _always_show_icon ? _rectangle.height() - _icon_size :
                _rectangle.height();

            ttlet text_x =
                _alignment == HorizontalAlignment::Center ? _rectangle.center() - text_width * 0.5f :
                _alignment == HorizontalAlignment::Left ? _rectangle.right() - text_width :
                _rectangle.left();

            ttlet text_y =
                _alignment == VerticalAlignment::Middle ? _rectangle.middle() - text_height * 0.5f :
                _alignment == VerticalAlignment::Bottom ? _rectangle.top() - text_height :
                _rectangle.bottom();
            // clang-format on

            ttlet text_rectangle = aarect{text_x, text_y, text_width, text_height};
            if (_alignment == HorizontalAlignment::Center) {
                _text_stencil->set_layout_parameters(text_rectangle);
            } else {
                _text_stencil->set_layout_parameters(text_rectangle, _base_line_position);
            }
        }
    }

    void draw(DrawContext context, bool use_context_color = false) noexcept override
    {
        if (_text_stencil) {
            _text_stencil->draw(context, use_context_color);
        }
        if (_icon_stencil) {
            _icon_stencil->draw(context, use_context_color);
        }
    }

private:
    bool _always_show_icon = false;
    TextStyle _style;
    float _icon_size;
    std::unique_ptr<image_stencil> _icon_stencil;
    std::unique_ptr<text_stencil> _text_stencil;
};

} // namespace tt
