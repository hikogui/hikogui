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

    [[nodiscard]] vec preferedExtent() const noexcept override;

    [[nodiscard]] float heightForWidth(float width) const noexcept override;

    [[nodiscard]] bool draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment, float middle) const noexcept override;

};

}