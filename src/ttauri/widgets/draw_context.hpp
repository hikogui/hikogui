// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../GFX/pipeline_box_vertex.hpp"
#include "../GFX/pipeline_image_vertex.hpp"
#include "../GFX/pipeline_SDF_vertex.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/matrix.hpp"
#include "../geometry/corner_shapes.hpp"
#include "../geometry/identity.hpp"
#include "../geometry/transform.hpp"
#include "../color/color.hpp"
#include "../color/quad_color.hpp"
#include "../vspan.hpp"
#include "widget_layout.hpp"

namespace tt::inline v1 {
class gfx_device;
class gfx_device_vulkan;
class shaped_text;
class font_glyph_ids;
struct paged_image;

/** The side where the border is drawn.
 */
enum class border_side {
    /** The border is drawn on the edge of a quad.
     */
    on,

    /** The border is drawn inside the edge of a quad.
     */
    inside,

    /** The border is drawn outside the edge of a quad.
     */
    outside
};

/** Draw context for drawing using the TTauri shaders.
 */
class draw_context {
public:
    gfx_device_vulkan &device;

    /** The frame buffer index of the image we are currently rendering.
     */
    size_t frame_buffer_index;

    /** This is the rectangle of the window that is being redrawn.
     */
    aarectangle scissor_rectangle;

    utc_nanoseconds display_time_point;

    draw_context(draw_context const &rhs) noexcept = default;
    draw_context(draw_context &&rhs) noexcept = default;
    draw_context &operator=(draw_context const &rhs) noexcept = default;
    draw_context &operator=(draw_context &&rhs) noexcept = default;
    ~draw_context() = default;

    draw_context(
        gfx_device_vulkan &device,
        size_t frame_buffer_index,
        aarectangle scissor_rectangle,
        vspan<pipeline_box::vertex> &boxVertices,
        vspan<pipeline_image::vertex> &imageVertices,
        vspan<pipeline_SDF::vertex> &sdfVertices,
        utc_nanoseconds display_time_point) noexcept;

    /** Draw a box with rounded corners.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param fill_color The fill color of the inside of the box.
     * @param border_color The line color of the border of the box.
     * @param border_width The line width of the border.
     * @param border_side The side of the edge where the border is drawn.
     * @param corner_radius The corner radii of each corner of the box.
     */
    void draw_box(
        widget_layout const &layout,
        quad const &box,
        quad_color const &fill_color,
        quad_color const &border_color,
        float border_width,
        tt::border_side border_side,
        tt::corner_shapes const &corner_radius = {}) const noexcept
    {
        return _draw_box(
            layout.window_clipping_rectangle(),
            layout.to_window * box,
            fill_color,
            border_color,
            border_width,
            border_side,
            corner_radius);
    }

    /** Draw a box with rounded corners.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param fill_color The fill color of the inside of the box.
     * @param border_color The line color of the border of the box.
     * @param border_width The line width of the border.
     * @param border_side The side of the edge where the border is drawn.
     * @param corner_radius The corner radii of each corner of the box.
     */
    void draw_box(
        widget_layout const &layout,
        aarectangle const &clipping_rectangle,
        quad const &box,
        quad_color const &fill_color,
        quad_color const &border_color,
        float border_width,
        tt::border_side border_side,
        tt::corner_shapes const &corner_radius = {}) const noexcept
    {
        return _draw_box(
            layout.window_clipping_rectangle(clipping_rectangle),
            layout.to_window * box,
            fill_color,
            border_color,
            border_width,
            border_side,
            corner_radius);
    }
    /** Draw a box with rounded corners without a border.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param fill_color The fill color of the inside of the box.
     * @param corner_radius The corner radii of each corner of the box.
     */
    void draw_box(
        widget_layout const &layout,
        quad const &box,
        quad_color const &fill_color,
        tt::corner_shapes const &corner_radius = {})
        const noexcept
    {
        return _draw_box(
            layout.window_clipping_rectangle(),
            layout.to_window * box,
            fill_color,
            fill_color,
            0.0f,
            border_side::on,
            corner_radius);
    }

    /** Draw a box with rounded corners without a border.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param clipping_rectangle A more narrow clipping rectangle than supplied by layout.
     * @param box The four points of the box to draw.
     * @param fill_color The fill color of the inside of the box.
     * @param corner_radius The corner radii of each corner of the box.
     */
    void draw_box(
        widget_layout const &layout,
        aarectangle const &clipping_rectangle,
        quad const &box,
        quad_color const &fill_color,
        tt::corner_shapes const &corner_radius = {}) const noexcept
    {
        return _draw_box(
            layout.window_clipping_rectangle(clipping_rectangle),
            layout.to_window * box,
            fill_color,
            fill_color,
            0.0f,
            border_side::on,
            corner_radius);
    }

    /** Draw an image
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param image The image to show.
     * @return True when the image was drawn, false if the image is not ready yet.
     *         Widgets may want to request a redraw if the image is not ready.
     */
    [[nodiscard]] bool draw_image(widget_layout const &layout, quad const &box, paged_image &image) const noexcept
    {
        return _draw_image(layout.window_clipping_rectangle(), layout.to_window * box, image);
    }

    /** Draw a glyph.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The size and position of the glyph.
     * @param color The color that the glyph should be drawn in.
     * @param glyph The glyphs to draw.
     */
    void
    draw_glyph(widget_layout const &layout, quad const &box, quad_color const &color, font_glyph_ids const &glyph) const noexcept
    {
        return _draw_glyph(layout.window_clipping_rectangle(), layout.to_window * box, color, glyph);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param color Text-color overriding the colors from the shaped_text.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const &layout, matrix3 const &transform, quad_color const &color, shaped_text const &text)
        const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(), layout.to_window * transform, text, color);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param clipping_rectangle A more narrow clipping rectangle than supplied by layout.
     * @param transform How to transform the shaped text relative to layout.
     * @param color Text-color overriding the colors from the shaped_text.
     * @param text The shaped text to draw.
     */
    void draw_text(
        widget_layout const &layout,
        aarectangle const &clipping_rectangle,
        matrix3 const &transform,
        quad_color const &color,
        shaped_text const &text
        ) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(clipping_rectangle), layout.to_window * transform, text, color);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const &layout, matrix3 const &transform, shaped_text const &text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(), layout.to_window * transform, text);
    }

    [[nodiscard]] friend bool overlaps(draw_context const &context, widget_layout const &layout) noexcept
    {
        return overlaps(context.scissor_rectangle, layout.window_clipping_rectangle());
    }

private:
    vspan<pipeline_box::vertex> *_box_vertices;
    vspan<pipeline_image::vertex> *_image_vertices;
    vspan<pipeline_SDF::vertex> *_sdf_vertices;

    void _draw_box(
        aarectangle const &clipping_rectangle,
        quad box,
        quad_color const &fill_color,
        quad_color const &border_color,
        float border_width,
        tt::border_side border_side,
        tt::corner_shapes corner_radius) const noexcept;

    void _draw_text(
        aarectangle const &clipping_rectangle,
        matrix3 const &transform,
        shaped_text const &text,
        std::optional<quad_color> color = {}) const noexcept;

    void _draw_glyph(aarectangle const &clipping_rectangle, quad const &box, quad_color const &color, font_glyph_ids const &glyph)
        const noexcept;

    [[nodiscard]] bool _draw_image(aarectangle const &clipping_rectangle, quad const &box, paged_image &image) const noexcept;
};

} // namespace tt::inline v1
