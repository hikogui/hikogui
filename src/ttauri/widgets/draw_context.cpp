// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "draw_context.hpp"
#include "../GFX/pipeline_box_device_shared.hpp"
#include "../GFX/pipeline_image_device_shared.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"
#include "../GFX/pipeline_image_image.hpp"
#include "../GFX/gfx_device_vulkan.hpp"
#include "../text/shaped_text.hpp"

namespace tt {

draw_context::draw_context(
    gfx_device_vulkan &device,
    size_t frame_buffer_index,
    aarectangle scissor_rectangle,
    vspan<pipeline_box::vertex> &boxVertices,
    vspan<pipeline_image::vertex> &imageVertices,
    vspan<pipeline_SDF::vertex> &sdfVertices,
    utc_nanoseconds display_time_point) noexcept :
    device(device),
    frame_buffer_index(frame_buffer_index),
    scissor_rectangle(scissor_rectangle),
    _box_vertices(&boxVertices),
    _image_vertices(&imageVertices),
    _sdf_vertices(&sdfVertices),
    display_time_point(display_time_point)
{
    _box_vertices->clear();
    _image_vertices->clear();
    _sdf_vertices->clear();
}

void draw_context::draw_box(
    widget_layout const &layout,
    quad box,
    quad_color fill_color,
    quad_color border_color,
    float border_width,
    tt::border_side border_side,
    tt::corner_shapes corner_radius) const noexcept
{
    // Expand or shrink the box and the corner radius.
    ttlet border_line_width = border_width * 0.5f;
    if (border_side == border_side::inside) {
        box = box - extent2{border_line_width, border_line_width};
        corner_radius = corner_radius - border_line_width;

    } else if (border_side == border_side::outside) {
        box = box + extent2{border_line_width, border_line_width};
        corner_radius = corner_radius + border_line_width;
    }

    if (_box_vertices->full()) {
        // Too many boxes where added, just don't draw them anymore.
        ++global_counter<"draw_box::overflow">;
        return;
    }

    pipeline_box::device_shared::place_vertices(
        *_box_vertices,
        bounding_rectangle(layout.to_window * layout.clipping_rectangle),
        layout.to_window * box,
        fill_color,
        border_color,
        border_width,
        corner_radius);
}

void draw_context::draw_image(widget_layout const &layout, pipeline_image::image &image, matrix3 image_transform) const noexcept
{
    tt_axiom(_image_vertices != nullptr);

    image.place_vertices(
        *_image_vertices, bounding_rectangle(layout.to_window * layout.clipping_rectangle), layout.to_window * image_transform);
}

void draw_context::draw_glyph(widget_layout const &layout, font_glyph_ids const &glyph, quad const &box, quad_color text_color)
    const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);
    ttlet pipeline = narrow_cast<gfx_device_vulkan &>(device).SDFPipeline.get();

    if (_sdf_vertices->full()) {
        ++global_counter<"draw_glyph::overflow">;
        return;
    }

    ttlet atlas_was_updated = pipeline->place_vertices(
        *_sdf_vertices,
        bounding_rectangle(layout.to_window * layout.clipping_rectangle),
        layout.to_window * box,
        glyph,
        text_color);

    if (atlas_was_updated) {
        pipeline->prepare_atlas_for_rendering();
    }
}

void draw_context::draw_text(
    widget_layout const &layout,
    shaped_text const &text,
    std::optional<quad_color> text_color,
    matrix3 transform) const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);
    ttlet pipeline = narrow_cast<gfx_device_vulkan &>(device).SDFPipeline.get();

    ttlet clipping_rectangle = bounding_rectangle(layout.to_window * layout.clipping_rectangle);
    ttlet to_window_transform = layout.to_window * transform;

    auto atlas_was_updated = false;
    for (ttlet &attr_glyph : text) {
        if (not is_visible(attr_glyph.general_category)) {
            continue;

        } else if (_sdf_vertices->full()) {
            ++global_counter<"draw_glyph::overflow">;
            break;
        }

        ttlet color = text_color ? *text_color : quad_color{attr_glyph.style.color};

        atlas_was_updated |= pipeline->place_vertices(
            *_sdf_vertices, clipping_rectangle, to_window_transform * attr_glyph.boundingBox(), attr_glyph.glyphs, color);
    }

    if (atlas_was_updated) {
        pipeline->prepare_atlas_for_rendering();
    }
}

} // namespace tt
