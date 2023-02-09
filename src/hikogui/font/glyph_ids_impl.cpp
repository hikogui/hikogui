// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font.hpp"
#include "../graphic_path.hpp"
#include "../log.hpp"

namespace hi::inline v1 {

[[nodiscard]] glyph_atlas_info &glyph_ids::atlas_info() const noexcept
{
    return _font->atlas_info(*this);
}

[[nodiscard]] std::pair<graphic_path, aarectangle> glyph_ids::get_path_and_bounding_box() const noexcept
{
    graphic_path path;
    auto bounding_box = aarectangle{};

    for (std::size_t i = 0; i < num_glyphs(); i++) {
        hilet glyph_id = (*this)[i];

        path += font().get_path(glyph_id);

        hilet glyph_metrics = font().get_metrics(glyph_id);

        if (i == 0) {
            bounding_box = glyph_metrics.bounding_rectangle;
        } else {
            bounding_box |= glyph_metrics.bounding_rectangle;
        }
    }

    return {path, bounding_box};
}

[[nodiscard]] aarectangle glyph_ids::get_bounding_box() const noexcept
{
    graphic_path path;
    auto bounding_box = aarectangle{};

    for (std::size_t i = 0; i < num_glyphs(); i++) {
        hilet glyph_id = (*this)[i];

        auto glyph_metrics = font().get_metrics(glyph_id);

        if (i == 0) {
            bounding_box = glyph_metrics.bounding_rectangle;
        } else {
            bounding_box |= glyph_metrics.bounding_rectangle;
        }
    }

    return bounding_box;
}

} // namespace hi::inline v1
