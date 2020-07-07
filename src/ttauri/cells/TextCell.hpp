// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Cells/Cell.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Text/TextStyle.hpp"
#include <string_view>

namespace tt {

class TextCell: public Cell {
    std::string text;
    TextStyle style;
    mutable ShapedText shapedText;

public:
    TextCell(std::string_view text, TextStyle style) noexcept;
    TextCell(std::string text, TextStyle style) noexcept;

    [[nodiscard]] vec preferredExtent() const noexcept override;

    [[nodiscard]] float heightForWidth(float width) const noexcept override;

    void draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment, float middle) const noexcept override;

};

}
