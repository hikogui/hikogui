// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil.hpp"
#include "../text/ShapedText.hpp"
#include "../text/TextStyle.hpp"
#include <string_view>

namespace tt {

class text_stencil : public stencil {
public:
    using super = stencil;

    text_stencil(Alignment alignment, std::u8string_view text, TextStyle style) noexcept;
    text_stencil(Alignment alignment, std::u8string text, TextStyle style) noexcept;

    [[nodiscard]] vec preferred_extent() noexcept override;

    void draw(DrawContext context, bool use_context_color = false) noexcept override;

private:
    std::u8string _text;
    TextStyle _style;
    ShapedText _shaped_text;
    mat _shaped_text_transform;
};

} // namespace tt
