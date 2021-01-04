// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "image_stencil.hpp"
#include "../GUI/PipelineImage_Image.hpp"
#include "../pixel_map.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include "../URL.hpp"

namespace tt {

class pixel_map_stencil : public image_stencil {
public:
    pixel_map_stencil(tt::alignment alignment, pixel_map<R16G16B16A16SFloat> &&pixel_map);
    pixel_map_stencil(tt::alignment alignment, pixel_map<R16G16B16A16SFloat> const &pixel_map);
    pixel_map_stencil(tt::alignment alignment, URL const &url);

    void draw(draw_context context, bool use_context_color=false) noexcept override;

private:
    pixel_map<R16G16B16A16SFloat> _pixel_map;
    PipelineImage::Image _backing;
    aarect _pixel_map_bounding_box;
    mat _pixel_map_transform;
};

}
