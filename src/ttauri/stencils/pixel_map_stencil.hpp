// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "image_stencil.hpp"
#include "../GUI/pipeline_image_image.hpp"
#include "../pixel_map.hpp"
#include "../color/sfloat_rgba16.hpp"
#include "../URL.hpp"

namespace tt {

class pixel_map_stencil : public image_stencil {
public:
    pixel_map_stencil(tt::alignment alignment, pixel_map<sfloat_rgba16> &&pixel_map);
    pixel_map_stencil(tt::alignment alignment, pixel_map<sfloat_rgba16> const &pixel_map);
    pixel_map_stencil(tt::alignment alignment, URL const &url);

    void draw(draw_context context, tt::color color, matrix3 transform) noexcept override;

private:
    pixel_map<sfloat_rgba16> _pixel_map;
    pipeline_image::Image _backing;
    aarect _pixel_map_bounding_box;
    matrix2 _pixel_map_transform;
};

}
