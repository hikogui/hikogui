// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_stencil.hpp"
#include "../GUI/draw_context.hpp"
#include "../codec/png.hpp"

namespace tt {

text_stencil::text_stencil(alignment alignment, std::u8string_view text, text_style style) noexcept :
    super(alignment), _text(text), _style(style), _shaped_text(text, style, 0.0f, alignment::top_left)
{
}

text_stencil::text_stencil(alignment alignment, std::u8string text, text_style style) noexcept :
    super(alignment), _text(std::move(text)), _style(style), _shaped_text(_text, style, 0.0f, alignment)
{
}

extent2 text_stencil::preferred_extent() noexcept
{
    return _shaped_text.preferred_extent;
}

void text_stencil::draw(draw_context context, tt::color color, matrix3 transform) noexcept
{
    auto data_is_modified = std::exchange(_data_is_modified, false);

    if (std::exchange(_size_is_modified, false) || data_is_modified) {
        _shaped_text = shaped_text(_text, _style, _rectangle.width(), _alignment);
        _position_is_modified = true;
    }

    if (std::exchange(_position_is_modified, false)) {
        _shaped_text_transform = _shaped_text.translate_base_line(point2{_rectangle.left(), _base_line_position});
    }

    context.draw_text(_shaped_text, color, transform * _shaped_text_transform);
}

} // namespace tt
