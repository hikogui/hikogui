// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "draw_context.hpp"
#include "../GFX/pipeline_box_device_shared.hpp"
#include "../GFX/pipeline_image_device_shared.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"
#include "../GFX/paged_image.hpp"
#include "../GFX/gfx_device_vulkan.hpp"
#include "../text/shaped_text.hpp"
#include "../text/text_shaper.hpp"
#include "../text/text_selection.hpp"

namespace tt::inline v1 {

draw_context::draw_context(
    gfx_device_vulkan &device,
    std::size_t frame_buffer_index,
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

void draw_context::_draw_box(
    aarectangle const &clipping_rectangle,
    quad box,
    quad_color const &fill_color,
    quad_color const &border_color,
    float border_width,
    tt::corner_radii corner_radius) const noexcept
{
    if (_box_vertices->full()) {
        // Too many boxes where added, just don't draw them anymore.
        ++global_counter<"draw_box::overflow">;
        return;
    }

    pipeline_box::device_shared::place_vertices(
        *_box_vertices, clipping_rectangle, box, fill_color, border_color, border_width, corner_radius);
}

[[nodiscard]] bool
draw_context::_draw_image(aarectangle const &clipping_rectangle, quad const &box, paged_image &image) const noexcept
{
    tt_axiom(_image_vertices != nullptr);

    if (image.state != paged_image::state_type::uploaded) {
        return false;
    }

    ttlet pipeline = down_cast<gfx_device_vulkan &>(device).imagePipeline.get();
    pipeline->place_vertices(*_image_vertices, clipping_rectangle, box, image);
    return true;
}

void draw_context::_draw_glyph(
    aarectangle const &clipping_rectangle,
    quad const &box,
    quad_color const &color,
    glyph_ids const &glyph) const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);
    ttlet pipeline = down_cast<gfx_device_vulkan &>(device).SDFPipeline.get();

    if (_sdf_vertices->full()) {
        _draw_box(clipping_rectangle, box, tt::color{1.0f, 0.0f, 1.0f}, tt::color{}, 0.0f, {});
        ++global_counter<"draw_glyph::overflow">;
        return;
    }

    ttlet atlas_was_updated = pipeline->place_vertices(*_sdf_vertices, clipping_rectangle, box, glyph, color);

    if (atlas_was_updated) {
        pipeline->prepare_atlas_for_rendering();
    }
}

void draw_context::_draw_text(
    aarectangle const &clipping_rectangle,
    matrix3 const &transform,
    shaped_text const &text,
    std::optional<quad_color> text_color) const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);
    ttlet pipeline = down_cast<gfx_device_vulkan &>(device).SDFPipeline.get();

    auto atlas_was_updated = false;
    for (ttlet &attr_glyph : text) {
        ttlet box = attr_glyph.boundingBox();
        ttlet color = text_color ? *text_color : quad_color{attr_glyph.style.color};

        if (not is_visible(attr_glyph.general_category)) {
            continue;

        } else if (_sdf_vertices->full()) {
            _draw_box(clipping_rectangle, box, tt::color{1.0f, 0.0f, 1.0f}, tt::color{}, 0.0f, {});
            ++global_counter<"draw_glyph::overflow">;
            break;
        }

        atlas_was_updated |=
            pipeline->place_vertices(*_sdf_vertices, clipping_rectangle, transform * box, attr_glyph.glyphs, color);
    }

    if (atlas_was_updated) {
        pipeline->prepare_atlas_for_rendering();
    }
}

void draw_context::_draw_text(
    aarectangle const &clipping_rectangle,
    matrix3 const &transform,
    text_shaper const &text,
    std::optional<quad_color> text_color) const noexcept
{
    tt_axiom(_sdf_vertices != nullptr);
    ttlet pipeline = down_cast<gfx_device_vulkan &>(device).SDFPipeline.get();

    auto atlas_was_updated = false;
    for (ttlet &c : text) {
        ttlet box = translate2{c.position} * c.metrics.bounding_rectangle;
        ttlet color = text_color ? *text_color : quad_color{c.style.color};

        tt_axiom(c.description != nullptr);
        if (not is_visible(c.description->general_category())) {
            continue;

        } else if (_sdf_vertices->full()) {
            _draw_box(clipping_rectangle, box, tt::color{1.0f, 0.0f, 1.0f}, tt::color{}, 0.0f, {});
            ++global_counter<"draw_glyph::overflow">;
            break;
        }

        atlas_was_updated |= pipeline->place_vertices(*_sdf_vertices, clipping_rectangle, transform * box, c.glyph, color);
    }

    if (atlas_was_updated) {
        pipeline->prepare_atlas_for_rendering();
    }
}

void draw_context::_draw_text_selection(
    aarectangle const &clipping_rectangle,
    matrix3 const &transform,
    text_shaper const &text,
    text_selection const &selection,
    tt::color color) const noexcept
{
    ttlet[first, last] = selection.selection_indices();
    ttlet first_ = text.begin() + first;
    ttlet last_ = text.begin() + last;
    tt_axiom(first_ <= text.end());
    tt_axiom(last_ <= text.end());
    tt_axiom(first_ <= last_);

    for (auto it = first_; it != last_; ++it) {
        _draw_box(clipping_rectangle, transform * it->rectangle, color, tt::color{}, 0.0f, {});
    }
}

void draw_context::_draw_text_insertion_cursor(
    aarectangle const &clipping_rectangle,
    matrix3 const &transform,
    text_shaper::char_const_iterator it,
    bool on_right,
    tt::color color,
    bool show_flag) const noexcept
{
    ttlet ltr = it->direction == unicode_bidi_class::L;

    ttlet bottom = std::floor(it->rectangle.bottom());
    ttlet top = std::ceil(it->rectangle.top());
    ttlet left = std::round((on_right ? it->rectangle.right() : it->rectangle.left()) - 0.5f);

    ttlet shape_I = aarectangle{point2{left, bottom}, point2{left + 1.0f, top}};
    _draw_box(clipping_rectangle, transform * shape_I, color, tt::color{}, 0.0f, {});

    if (show_flag) {
        ttlet shape_flag = ltr ? aarectangle{point2{left + 1.0f, top - 1.0f}, point2{left + 3.0f, top}} :
                                 aarectangle{point2{left - 2.0f, top - 1.0f}, point2{left, top}};

        _draw_box(clipping_rectangle, transform * shape_flag, color, tt::color{}, 0.0f, {});
    }
}

void draw_context::_draw_text_overwrite_cursor(
    aarectangle const &clipping_rectangle,
    matrix3 const &transform,
    text_shaper::char_const_iterator it,
    tt::color color) const noexcept
{
    ttlet box = ceil(it->rectangle) + 0.5f;
    _draw_box(clipping_rectangle, transform * box, tt::color{}, color, 1.0f, {});
}

void draw_context::_draw_text_cursors(
    aarectangle const &clipping_rectangle,
    matrix3 const &transform,
    text_shaper const &text,
    text_cursor primary_cursor,
    tt::color primary_color,
    tt::color secondary_color,
    bool insertion_mode) const noexcept
{
    auto draw_flags = false;

    if (text.size() == 0) {
        // XXX handle cursor on empty text.
        return;
    }

    tt_axiom(primary_cursor.index() < text.size());

    if (not insertion_mode and not primary_cursor.end_of_text(text.size())) {
        if (primary_cursor.after()) {
            primary_cursor = primary_cursor.neighbour();
        }
        return _draw_text_overwrite_cursor(clipping_rectangle, transform, text.begin() + primary_cursor.index(), primary_color);
    }

    // calculate the position of the primary cursor.
    ttlet primary_it = text.begin() + primary_cursor.index();
    ttlet primary_ltr = primary_it->direction == unicode_bidi_class::L;
    ttlet primary_is_on_right = primary_ltr == primary_cursor.after();
    ttlet primary_is_on_left = not primary_is_on_right;

    do {
        if (primary_cursor.start_of_text()) {
            // Don't draw secondary cursor which would be on the other edge of the text-field.
            break;
        }

        ttlet secondary_cursor = primary_cursor.neighbour();
        ttlet secondary_it = text.begin() + secondary_cursor.index();
        if (secondary_it == text.end()) {
            // Secondary cursor is at end-of-text.
            break;
        }

        ttlet secondary_ltr = secondary_it->direction == unicode_bidi_class::L;
        ttlet secondary_is_on_right = secondary_ltr == secondary_cursor.after();
        ttlet secondary_is_on_left = not secondary_is_on_right;

        if (primary_is_on_right and secondary_is_on_left and text.move_right_char(primary_it) == secondary_it) {
            // The secondary character is right of primary character, and the cursors are touching.
            break;
        } else if (primary_is_on_left and secondary_is_on_right and text.move_left_char(primary_it) == secondary_it) {
            // The secondary character is left of primary character, and the cursors are touching.
            break;
        }

        draw_flags = true;
        _draw_text_insertion_cursor(
            clipping_rectangle, transform, secondary_it, secondary_is_on_right, secondary_color, draw_flags);
    } while (false);

    _draw_text_insertion_cursor(
        clipping_rectangle, transform, primary_it, primary_is_on_right, primary_color, draw_flags);
}

} // namespace tt::inline v1
