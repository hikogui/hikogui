// Copyright 2020 Pokitec
// All rights reserved.

#include "text_stencil.hpp"
#include "../GUI/Window.hpp"
#include "../GUI/DrawContext.hpp"
#include "../encoding/png.hpp"

namespace tt {

text_stencil::text_stencil(alignment alignment, std::u8string_view text, TextStyle style) noexcept :
    super(alignment), _text(text), _style(style), _shaped_text(text, style, 0.0f, alignment::top_left)
{
}

text_stencil::text_stencil(alignment alignment, std::u8string text, TextStyle style) noexcept :
    super(alignment), _text(std::move(text)), _style(style), _shaped_text(_text, style, 0.0f, alignment)
{
}

vec text_stencil::preferred_extent() noexcept
{
    return _shaped_text.preferred_extent;
}

void text_stencil::draw(DrawContext context, bool use_context_color) noexcept
{
    auto modified = std::exchange(_data_is_modified, false);
    modified |= std::exchange(_layout_is_modified, false);

    if (modified) {
        _shaped_text = ShapedText(_text, _style, _rectangle.width(), _alignment);

        _shaped_text_transform = _shaped_text.TMiddle(vec{_rectangle.x(), _base_line_position});
    }

    context.transform = context.transform * _shaped_text_transform;
    context.drawText(_shaped_text, use_context_color);
}

} // namespace tt
