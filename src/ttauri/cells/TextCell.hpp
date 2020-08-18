// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Cell.hpp"
#include "../text/ShapedText.hpp"
#include "../text/TextStyle.hpp"
#include <string_view>

namespace tt {

class TextCell: public Cell {
    std::u8string text;
    TextStyle style;
    mutable ShapedText shapedText;

public:
    TextCell(std::u8string_view text, TextStyle style) noexcept;
    TextCell(std::u8string text, TextStyle style) noexcept;

    [[nodiscard]] vec preferredExtent() const noexcept override;

    [[nodiscard]] float heightForWidth(float width) const noexcept override;

    void draw(
        DrawContext const &drawContext,
        aarect rectangle,
        Alignment alignment,
        float middle=std::numeric_limits<float>::max(),
        bool useContextColor=false
    ) const noexcept override;

};

}
