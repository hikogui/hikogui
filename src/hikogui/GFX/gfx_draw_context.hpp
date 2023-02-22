

#pragma once

#include "subpixel_orientation.hpp"
#include "../geometry/module.hpp"
#include "../vector_span.hpp"
#include <cstdlib>

namespace hi { inline namespace v1 {
namespace pipeline_box {
struct vertex;
}
namespace pipeline_image {
struct vertex;
}
namespace pipeline_SDF {
struct vertex;
}
namespace pipeline_alpha {
struct vertex;
}
class gfx_device;


struct gfx_draw_context {
    gfx_device *device = nullptr;
    vector_span<pipeline_box::vertex> *box_vertices;
    vector_span<pipeline_image::vertex> *image_vertices;
    vector_span<pipeline_SDF::vertex> *sdf_vertices;
    vector_span<pipeline_alpha::vertex> *alpha_vertices;
    aarectanglei scissor_rectangle = {};
    size_t frame_buffer_index = 0;
    hi::subpixel_orientation subpixel_orientation = hi::subpixel_orientation::unknown;
};

}} // namespace hi::v1
