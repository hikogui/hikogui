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
    layout_context const &layout,
    rectangle box,
    color fill_color,
    color line_color,
    float line_width,
    tt::corner_shapes corner_shapes) const noexcept
{
    tt_axiom(_box_vertices != nullptr);

    pipeline_box::device_shared::place_vertices(
        *_box_vertices,
        bounding_rectangle(layout.to_window * layout.clipping_rectangle),
        layout.to_window * box,
        fill_color,
        line_color,
        line_width,
        corner_shapes);
}

void draw_context::draw_box(
    layout_context const &layout,
    rectangle box,
    color fill_color,
    color line_color,
    tt::corner_shapes corner_shapes) const noexcept
{
    draw_box(layout, box, fill_color, line_color, 1.0, corner_shapes);
}

void draw_context::draw_box(layout_context const &layout, rectangle box, color fill_color, tt::corner_shapes corner_shapes)
    const noexcept
{
    draw_box(layout, box, fill_color, fill_color, 0.0, corner_shapes);
}

void draw_context::draw_box(layout_context const &layout, rectangle box, color fill_color) const noexcept
{
    draw_box(layout, box, fill_color, fill_color, 0.0, tt::corner_shapes{});
}

void draw_context::draw_box_with_border_inside(
    layout_context const &layout,
    rectangle rectangle,
    color fill_color,
    color line_color,
    float line_width,
    tt::corner_shapes corner_shapes) const noexcept
{
    tt_axiom(_box_vertices != nullptr);

    ttlet shrink_value = line_width * 0.5f;

    ttlet new_rectangle = rectangle - shrink_value;

    ttlet new_corner_shapes = corner_shapes - shrink_value;

    pipeline_box::device_shared::place_vertices(
        *_box_vertices,
        bounding_rectangle(layout.to_window * layout.clipping_rectangle),
        layout.to_window * new_rectangle,
        fill_color,
        line_color,
        line_width,
        new_corner_shapes);
}

void draw_context::draw_box_with_border_inside(
    layout_context const &layout,
    rectangle rectangle,
    color fill_color,
    color line_color,
    tt::corner_shapes corner_shapes) const noexcept
{
    draw_box_with_border_inside(layout, rectangle, fill_color, line_color, 1.0, corner_shapes);
}

void draw_context::draw_box_with_border_outside(
    layout_context const &layout,
    rectangle rectangle,
    color fill_color,
    color line_color,
    float line_width,
    tt::corner_shapes corner_shapes) const noexcept
{
    tt_axiom(_box_vertices != nullptr);

    ttlet expand_value = line_width * 0.5f;

    ttlet new_rectangle = rectangle + expand_value;

    ttlet new_corner_shapes = corner_shapes + expand_value;

    pipeline_box::device_shared::place_vertices(
        *_box_vertices,
        bounding_rectangle(layout.to_window * layout.clipping_rectangle),
        layout.to_window * new_rectangle,
        fill_color,
        line_color,
        line_width,
        new_corner_shapes);
}

void draw_context::draw_box_with_border_outside(
    layout_context const &layout,
    rectangle rectangle,
    color fill_color,
    color line_color,
    tt::corner_shapes corner_shapes) const noexcept
{
    draw_box_with_border_outside(layout, rectangle, fill_color, line_color, 1.0, corner_shapes);
}

void draw_context::draw_image(layout_context const &layout, pipeline_image::image &image, matrix3 image_transform) const noexcept
{
    tt_axiom(_image_vertices != nullptr);

    image.place_vertices(
        *_image_vertices, bounding_rectangle(layout.to_window * layout.clipping_rectangle), layout.to_window * image_transform);
}

void draw_context::draw_text(
    layout_context const &layout,
    shaped_text const &text,
    std::optional<color> text_color,
    matrix3 transform) const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);

    if (text_color) {
        narrow_cast<gfx_device_vulkan &>(device).SDFPipeline->place_vertices(
            *_sdf_vertices,
            bounding_rectangle(layout.to_window * layout.clipping_rectangle),
            layout.to_window * transform,
            text,
            *text_color);
    } else {
        narrow_cast<gfx_device_vulkan &>(device).SDFPipeline->place_vertices(
            *_sdf_vertices, bounding_rectangle(layout.to_window * layout.clipping_rectangle), layout.to_window * transform, text);
    }
}

void draw_context::draw_glyph(
    layout_context const &layout,
    font_glyph_ids const &glyph,
    quad const &box,
    color text_color) const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);

    narrow_cast<gfx_device_vulkan &>(device).SDFPipeline->place_vertices(
        *_sdf_vertices,
        bounding_rectangle(layout.to_window * layout.clipping_rectangle),
        layout.to_window * box,
        glyph,
        text_color);
}

} // namespace tt
