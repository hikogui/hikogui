// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "draw_context.hpp"
#include "pipeline_flat_device_shared.hpp"
#include "pipeline_box_device_shared.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "pipeline_image_image.hpp"
#include "gfx_device_vulkan.hpp"
#include "../text/shaped_text.hpp"

namespace tt {

draw_context::draw_context(
    gfx_device_vulkan &device,
    size_t frame_buffer_index,
    extent2 surface_size,
    aarectangle scissor_rectangle,
    vspan<pipeline_flat::vertex> &flatVertices,
    vspan<pipeline_box::vertex> &boxVertices,
    vspan<pipeline_image::vertex> &imageVertices,
    vspan<pipeline_SDF::vertex> &sdfVertices) noexcept :
    _device(device),
    _frame_buffer_index(frame_buffer_index),
    _scissor_rectangle(scissor_rectangle),
    _flat_vertices(&flatVertices),
    _box_vertices(&boxVertices),
    _image_vertices(&imageVertices),
    _sdf_vertices(&sdfVertices),
    _clipping_rectangle(surface_size)
{
    _flat_vertices->clear();
    _box_vertices->clear();
    _image_vertices->clear();
    _sdf_vertices->clear();
}

[[nodiscard]] draw_context
draw_context::make_child_context(matrix3 parent_to_local, matrix3 local_to_window, aarectangle clipping_rectangle) const noexcept
{
    auto new_context = *this;
    new_context._scissor_rectangle = aarectangle{parent_to_local * this->_scissor_rectangle};
    new_context._clipping_rectangle = clipping_rectangle;
    new_context._transform = local_to_window;
    return new_context;
}

[[nodiscard]] size_t draw_context::frame_buffer_index() const noexcept
{
    return _frame_buffer_index;
}

[[nodiscard]] aarectangle draw_context::scissor_rectangle() const noexcept
{
    return _scissor_rectangle;
}

[[nodiscard]] aarectangle draw_context::clipping_rectangle() const noexcept
{
    return _clipping_rectangle;
}

void draw_context::set_clipping_rectangle(aarectangle clipping_rectangle) noexcept
{
    _clipping_rectangle = clipping_rectangle;
}

[[nodiscard]] matrix3 draw_context::transform() const noexcept
{
    return _transform;
}

gfx_device &draw_context::device() const noexcept
{
    return _device;
}

void draw_context::draw_filled_quad(point3 p1, point3 p2, point3 p3, point3 p4, color fill_color) const noexcept
{
    tt_axiom(_flat_vertices != nullptr);
    _flat_vertices->emplace_back(aarectangle{_transform * _clipping_rectangle}, _transform * p1, fill_color);
    _flat_vertices->emplace_back(aarectangle{_transform * _clipping_rectangle}, _transform * p2, fill_color);
    _flat_vertices->emplace_back(aarectangle{_transform * _clipping_rectangle}, _transform * p3, fill_color);
    _flat_vertices->emplace_back(aarectangle{_transform * _clipping_rectangle}, _transform * p4, fill_color);
}

void draw_context::draw_filled_quad(rectangle r, color fill_color) const noexcept
{
    draw_filled_quad(get<0>(r), get<1>(r), get<2>(r), get<3>(r), fill_color);
}

void draw_context::draw_box(
    rectangle box,
    color fill_color,
    color line_color,
    float line_width,
    tt::corner_shapes corner_shapes) const noexcept
{
    tt_axiom(_box_vertices != nullptr);

    pipeline_box::device_shared::place_vertices(
        *_box_vertices,
        aarectangle{_transform * _clipping_rectangle},
        _transform * box,
        fill_color,
        line_color,
        line_width,
        corner_shapes);
}

void draw_context::draw_box(rectangle box, color fill_color, color line_color, tt::corner_shapes corner_shapes) const noexcept
{
    draw_box(box, fill_color, line_color, 1.0, corner_shapes);
}

void draw_context::draw_box(rectangle box, color fill_color, tt::corner_shapes corner_shapes) const noexcept
{
    draw_box(box, fill_color, fill_color, 0.0, corner_shapes);
}

void draw_context::draw_box_with_border_inside(
    rectangle rectangle,
    color fill_color,
    color line_color,
    float line_width,
    tt::corner_shapes corner_shapes) const noexcept
{
    tt_axiom(_box_vertices != nullptr);

    ttlet shrink_value = line_width * 0.5f;

    ttlet new_rectangle = shrink(rectangle, shrink_value);

    ttlet new_corner_shapes = corner_shapes - shrink_value;

    pipeline_box::device_shared::place_vertices(
        *_box_vertices,
        aarectangle{_transform * _clipping_rectangle},
        _transform * new_rectangle,
        fill_color,
        line_color,
        line_width,
        new_corner_shapes);
}

void draw_context::draw_box_with_border_inside(
    rectangle rectangle,
    color fill_color,
    color line_color,
    tt::corner_shapes corner_shapes)
    const noexcept
{
    draw_box_with_border_inside(rectangle, fill_color, line_color, 1.0, corner_shapes);
}

void draw_context::draw_box_with_border_outside(
    rectangle rectangle,
    color fill_color,
    color line_color,
    float line_width,
    tt::corner_shapes corner_shapes) const noexcept
{
    tt_axiom(_box_vertices != nullptr);

    ttlet expand_value = line_width * 0.5f;

    ttlet new_rectangle = expand(rectangle, expand_value);

    ttlet new_corner_shapes = corner_shapes + expand_value;

    pipeline_box::device_shared::place_vertices(
        *_box_vertices,
        aarectangle{_transform * _clipping_rectangle},
        _transform * new_rectangle,
        fill_color,
        line_color,
        line_width,
        new_corner_shapes);
}

void draw_context::draw_box_with_border_outside(
    rectangle rectangle,
    color fill_color,
    color line_color,
    tt::corner_shapes corner_shapes)
    const noexcept
{
    draw_box_with_border_outside(rectangle, fill_color, line_color, 1.0, corner_shapes);
}

void draw_context::draw_image(pipeline_image::image &image, matrix3 image_transform) const noexcept
{
    tt_axiom(_image_vertices != nullptr);

    image.place_vertices(*_image_vertices, aarectangle{_transform * _clipping_rectangle}, _transform * image_transform);
}

void draw_context::draw_text(shaped_text const &text, std::optional<color> text_color, matrix3 transform)
    const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);

    if (text_color) {
        narrow_cast<gfx_device_vulkan &>(device()).SDFPipeline->place_vertices(
            *_sdf_vertices, aarectangle{_transform * _clipping_rectangle}, _transform * transform, text, *text_color);
    } else {
        narrow_cast<gfx_device_vulkan &>(device()).SDFPipeline->place_vertices(
            *_sdf_vertices, aarectangle{_transform * _clipping_rectangle}, _transform * transform, text);
    }
}

void draw_context::draw_glyph(font_glyph_ids const &glyph, float glyph_size, rectangle box, color text_color) const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);

    narrow_cast<gfx_device_vulkan &>(device()).SDFPipeline->place_vertices(
        *_sdf_vertices, aarectangle{_transform * _clipping_rectangle}, _transform * box, glyph, glyph_size, text_color);
}

}
