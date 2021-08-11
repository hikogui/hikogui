// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "attributed_glyph.hpp"
#include "font.hpp"
#include "../graphic_path.hpp"
#include "../logger.hpp"

namespace tt {

[[nodiscard]] std::pair<graphic_path, aarectangle> font_glyph_ids::get_path_and_bounding_box() const noexcept
{
    graphic_path path;
    auto boundingBox = aarectangle{};

    for (ssize_t i = 0; i < std::ssize(*this); i++) {
        ttlet glyph_id = (*this)[i];

        graphic_path glyph_path;
        if (not font().load_glyph(glyph_id, glyph_path)) {
            tt_log_error(
                "Could not load glyph {} in font {} - {}",
                static_cast<int>(glyph_id),
                font().family_name,
                font().sub_family_name);
        }
        path += glyph_path;

        glyph_metrics glyph_metrics;
        if (not font().load_glyph_metrics(glyph_id, glyph_metrics)) {
            tt_log_error(
                "Could not load glyph-metrics {} in font {} - {}",
                static_cast<int>(glyph_id),
                font().family_name,
                font().sub_family_name);
        }

        if (i == 0) {
            boundingBox = glyph_metrics.boundingBox;
        } else {
            boundingBox |= glyph_metrics.boundingBox;
        }
    }

    return {path, boundingBox};
}

[[nodiscard]] aarectangle font_glyph_ids::get_bounding_box() const noexcept
{
    graphic_path path;
    auto boundingBox = aarectangle{};

    for (ssize_t i = 0; i < std::ssize(*this); i++) {
        ttlet glyph_id = (*this)[i];

        glyph_metrics glyph_metrics;
        if (not font().load_glyph_metrics(glyph_id, glyph_metrics)) {
            tt_log_error(
                "Could not load glyph-metrics {} in font {} - {}",
                static_cast<int>(glyph_id),
                font().family_name,
                font().sub_family_name);
        }

        if (i == 0) {
            boundingBox = glyph_metrics.boundingBox;
        } else {
            boundingBox |= glyph_metrics.boundingBox;
        }
    }

    return boundingBox;
}

} // namespace tt