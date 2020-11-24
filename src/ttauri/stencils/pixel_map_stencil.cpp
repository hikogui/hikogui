// Copyright 2020 Pokitec
// All rights reserved.

#include "pixel_map_stencil.hpp"
#include "../GUI/draw_context.hpp"
#include "../encoding/png.hpp"

namespace tt {

pixel_map_stencil::pixel_map_stencil(tt::alignment alignment, PixelMap<R16G16B16A16SFloat> &&pixel_map) :
    image_stencil(alignment), _pixel_map(std::move(pixel_map))
{
}

pixel_map_stencil::pixel_map_stencil(tt::alignment alignment, PixelMap<R16G16B16A16SFloat> const &pixel_map) :
    image_stencil(alignment), _pixel_map(pixel_map.copy())
{
}

pixel_map_stencil::pixel_map_stencil(tt::alignment alignment, URL const &url) : pixel_map_stencil(alignment, png::load(url)) {}

void pixel_map_stencil::draw(draw_context context, bool use_context_color) noexcept
{
    if (std::exchange(_data_is_modified, false)) {
        _backing = narrow_cast<gui_device_vulkan&>(context.device()).imagePipeline->makeImage(_pixel_map.extent());
        _backing.upload(_pixel_map);
        _size_is_modified = true;
        _position_is_modified = true;
    }

    auto layout_is_modified = std::exchange(_size_is_modified, false);
    layout_is_modified |= std::exchange(_position_is_modified, false);
    if (layout_is_modified) {
        _pixel_map_bounding_box = aarect{_backing.extent};
        _pixel_map_transform = mat::uniform2D_scale_and_translate(_rectangle, _pixel_map_bounding_box, _alignment);
    }

    context.transform = context.transform * _pixel_map_transform;

    switch (_backing.state) {
    case PipelineImage::Image::State::Drawing: context.window().request_redraw(context.clipping_rectangle); break;
    case PipelineImage::Image::State::Uploaded: context.draw_image(_backing); break;
    default:;
    }
}

} // namespace tt
