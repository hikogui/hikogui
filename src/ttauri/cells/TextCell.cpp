// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/cells/TextCell.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/DrawContext.hpp"
#include "ttauri/png.hpp"

namespace tt {

TextCell::TextCell(std::string_view text, TextStyle style) noexcept :
    text(text), style(style), shapedText(text, style, 0.0f, Alignment::TopLeft) {}

TextCell::TextCell(std::string text, TextStyle style) noexcept :
    text(std::move(text)), style(style), shapedText(this->text, style, 0.0f, Alignment::TopLeft) {}

vec TextCell::preferredExtent() const noexcept
{
    return shapedText.preferredExtent;
}

[[nodiscard]] float TextCell::heightForWidth(float width) const noexcept
{
    if (width != shapedText.width) {
        shapedText = ShapedText(text, style, width, shapedText.alignment);
    }
    return shapedText.boundingBox.height();
}

void TextCell::draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment, float middle) const noexcept
{
    if (modified || rectangle.width() != shapedText.width || alignment != shapedText.alignment) {
        shapedText = ShapedText(text, style, rectangle.width(), alignment);
        modified = false;
    }

    auto context = drawContext;
    if (middle == std::numeric_limits<float>::max()) {
        context.transform =
            context.transform *
            shapedText.T(rectangle);
    } else {
        context.transform =
            context.transform *
            shapedText.TMiddle(vec{rectangle.x(), middle});
    }

    context.drawText(shapedText);
}

}
