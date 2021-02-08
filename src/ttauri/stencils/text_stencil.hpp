// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil.hpp"
#include "../text/shaped_text.hpp"
#include "../text/text_style.hpp"
#include <string_view>

namespace tt {

class text_stencil : public stencil {
public:
    using super = stencil;

    text_stencil(alignment alignment, std::u8string_view text, text_style style) noexcept;
    text_stencil(alignment alignment, std::u8string text, text_style style) noexcept;

    [[nodiscard]] f32x4 preferred_extent() noexcept override;

    void draw(draw_context context, bool use_context_color = false) noexcept override;

private:
    std::u8string _text;
    text_style _style;
    shaped_text _shaped_text;
    mat _shaped_text_transform;
};

} // namespace tt
