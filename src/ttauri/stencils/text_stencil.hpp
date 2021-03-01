// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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

    [[nodiscard]] extent2 preferred_extent() noexcept override;

    void draw(draw_context context, tt::color color, matrix3 transform) noexcept override;

private:
    std::u8string _text;
    text_style _style;
    shaped_text _shaped_text;
    matrix2 _shaped_text_transform;
};

} // namespace tt
