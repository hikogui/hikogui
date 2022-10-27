// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_box_vertex.hpp"
#include "pipeline_image_vertex.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "pipeline_alpha_vertex.hpp"
#include "subpixel_orientation.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/matrix.hpp"
#include "../geometry/corner_radii.hpp"
#include "../geometry/identity.hpp"
#include "../geometry/transform.hpp"
#include "../geometry/circle.hpp"
#include "../geometry/line_end_cap.hpp"
#include "../text/text_cursor.hpp"
#include "../text/text_selection.hpp"
#include "../text/text_shaper.hpp"
#include "../color/color.hpp"
#include "../color/quad_color.hpp"
#include "../widgets/widget_layout.hpp"
#include "../vector_span.hpp"

namespace hi::inline v1 {
class gfx_device;
class gfx_device_vulkan;
class shaped_text;
class glyph_ids;
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

/** Draw context for drawing using the HikoGUI shaders.
 */
class draw_context {
public:
    gfx_device_vulkan& device;

    /** The frame buffer index of the image we are currently rendering.
     */
    std::size_t frame_buffer_index;

    /** This is the rectangle of the window that is being redrawn.
     */
    aarectangle scissor_rectangle;

    /** The background color to clear the window with.
     */
    color background_color;

    /** The subpixel orientation for rendering glyphs.
     */
    hi::subpixel_orientation subpixel_orientation;

    /** The tone-mapper's saturation.
     */
    float saturation;

    /** The time when the drawing will appear on the screen.
     */
    utc_nanoseconds display_time_point;

    draw_context(draw_context const& rhs) noexcept = default;
    draw_context(draw_context&& rhs) noexcept = default;
    draw_context& operator=(draw_context const& rhs) noexcept = default;
    draw_context& operator=(draw_context&& rhs) noexcept = default;
    ~draw_context() = default;

    draw_context(
        gfx_device_vulkan& device,
        vector_span<pipeline_box::vertex>& box_vertices,
        vector_span<pipeline_image::vertex>& image_vertices,
        vector_span<pipeline_SDF::vertex>& sdf_vertices,
        vector_span<pipeline_alpha::vertex>& alpha_vertices) noexcept;

    /** Check if the draw_context should be used for rendering.
     */
    operator bool() const noexcept
    {
        return frame_buffer_index != std::numeric_limits<size_t>::max();
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
        widget_layout const& layout,
        quad const& box,
        quad_color const& fill_color,
        quad_color const& border_color,
        float border_width,
        hi::border_side border_side,
        hi::corner_radii const& corner_radius = {}) const noexcept
    {
        // clang-format off
        hilet border_radius = border_width * 0.5f;
        hilet box_ =
            border_side == hi::border_side::inside ? box - border_radius :
            border_side == hi::border_side::outside ? box + border_radius :
            box;
        hilet corner_radius_ =
            border_side == hi::border_side::inside ? corner_radius - border_radius :
            border_side == hi::border_side::outside ? corner_radius + border_radius :
            corner_radius;
        // clang-format on

        return _draw_box(
            layout.window_clipping_rectangle(),
            layout.to_window * box_,
            fill_color,
            border_color,
            layout.to_window * border_width,
            layout.to_window * corner_radius_);
    }

    /** Draw a box with rounded corners.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param clipping_rectangle The rectangle into which to draw the box.
     * @param box The four points of the box to draw.
     * @param fill_color The fill color of the inside of the box.
     * @param border_color The line color of the border of the box.
     * @param border_width The line width of the border.
     * @param border_side The side of the edge where the border is drawn.
     * @param corner_radius The corner radii of each corner of the box.
     */
    void draw_box(
        widget_layout const& layout,
        aarectangle const& clipping_rectangle,
        quad const& box,
        quad_color const& fill_color,
        quad_color const& border_color,
        float border_width,
        hi::border_side border_side,
        hi::corner_radii const& corner_radius = {}) const noexcept
    {
        // clang-format off
        hilet border_radius = border_width * 0.5f;
        hilet box_ =
            border_side == hi::border_side::inside ? box - border_radius :
            border_side == hi::border_side::outside ? box + border_radius :
            box;
        hilet corner_radius_ =
            border_side == hi::border_side::inside ? corner_radius - border_radius :
            border_side == hi::border_side::outside ? corner_radius + border_radius :
            corner_radius;
        // clang-format on

        return _draw_box(
            layout.window_clipping_rectangle(clipping_rectangle),
            layout.to_window * box_,
            fill_color,
            border_color,
            layout.to_window * border_width,
            layout.to_window * corner_radius_);
    }
    /** Draw a box with rounded corners without a border.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param fill_color The fill color of the inside of the box.
     * @param corner_radius The corner radii of each corner of the box.
     */
    void draw_box(
        widget_layout const& layout,
        quad const& box,
        quad_color const& fill_color,
        hi::corner_radii const& corner_radius = {}) const noexcept
    {
        return _draw_box(
            layout.window_clipping_rectangle(),
            layout.to_window * box,
            fill_color,
            fill_color,
            0.0f,
            layout.to_window * corner_radius);
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
        widget_layout const& layout,
        aarectangle const& clipping_rectangle,
        quad const& box,
        quad_color const& fill_color,
        hi::corner_radii const& corner_radius = {}) const noexcept
    {
        return _draw_box(
            layout.window_clipping_rectangle(clipping_rectangle),
            layout.to_window * box,
            fill_color,
            fill_color,
            0.0f,
            layout.to_window * corner_radius);
    }

    [[nodiscard]] constexpr static rectangle
    make_rectangle(line_segment const& line, float width, line_end_cap c1, line_end_cap c2) noexcept
    {
        auto right = line.direction();

        hilet radius = width * 0.5f;
        hilet n = normal(right, 0.0f);
        hilet up = n * width;
        hilet t = normalize(right);

        auto origin = line.origin() - n * radius;

        // Extend the line by the radius for rounded end-caps.
        hilet radius_offset = t * radius;
        if (c1 == line_end_cap::round) {
            origin -= radius_offset;
            right += radius_offset;
        }
        if (c2 == line_end_cap::round) {
            right += radius_offset;
        }

        return rectangle{origin, right, up};
    }

    [[nodiscard]] constexpr static corner_radii make_corner_radii(float width, line_end_cap c1, line_end_cap c2) noexcept
    {
        auto r = f32x4::broadcast(width * 0.5f);

        if (c1 == line_end_cap::flat) {
            r = zero<0b0101>(r);
        }
        if (c2 == line_end_cap::flat) {
            r = zero<0b1010>(r);
        }

        return corner_radii{r};
    }

    void draw_line(
        widget_layout const& layout,
        line_segment const& line,
        float width,
        quad_color const& fill_color,
        line_end_cap c1 = line_end_cap::flat,
        line_end_cap c2 = line_end_cap::flat) const noexcept
    {
        hilet line_ = layout.to_window * line;
        hilet width_ = layout.to_window * width;

        hilet box = make_rectangle(line_, width_, c1, c2);
        hilet corners = make_corner_radii(width_, c1, c2);

        return _draw_box(layout.window_clipping_rectangle(), box, fill_color, fill_color, 0.0f, corners);
    }

    void draw_line(
        widget_layout const& layout,
        aarectangle const& clipping_rectangle,
        line_segment const& line,
        float width,
        quad_color const& fill_color,
        line_end_cap c1 = line_end_cap::flat,
        line_end_cap c2 = line_end_cap::flat) const noexcept
    {
        hi_assert(width != 0.0f);
        hilet line_ = layout.to_window * line;
        hilet width_ = layout.to_window * width;

        hilet box = make_rectangle(line_, width_, c1, c2);
        hilet corners = make_corner_radii(width_, c1, c2);

        return _draw_box(layout.window_clipping_rectangle(clipping_rectangle), box, fill_color, fill_color, 0.0f, corners);
    }

    [[nodiscard]] constexpr static rectangle make_rectangle(hi::circle const& circle) noexcept
    {
        hilet circle_ = f32x4{circle};
        hilet origin = point3{circle_.xyz1() - circle_.ww00()};
        hilet right = vector3{circle_.w000() * 2.0f};
        hilet up = vector3{circle_._0w00() * 2.0f};
        return rectangle{origin, right, up};
    }

    [[nodiscard]] constexpr static corner_radii make_corner_radii(hi::circle const& circle) noexcept
    {
        return corner_radii{f32x4{circle}.wwww()};
    }

    void draw_circle(widget_layout const& layout, hi::circle const& circle, quad_color const& fill_color) const noexcept
    {
        hilet box = layout.to_window * make_rectangle(circle);
        hilet corners = layout.to_window * make_corner_radii(circle);
        return _draw_box(layout.window_clipping_rectangle(), box, fill_color, fill_color, 0.0f, corners);
    }

    void draw_circle(
        widget_layout const& layout,
        aarectangle const clipping_rectangle,
        hi::circle const& circle,
        quad_color const& fill_color) const
    {
        hilet box = layout.to_window * make_rectangle(circle);
        hilet corners = layout.to_window * make_corner_radii(circle);
        return _draw_box(layout.window_clipping_rectangle(clipping_rectangle), box, fill_color, fill_color, 0.0f, corners);
    }

    void draw_circle(
        widget_layout const& layout,
        hi::circle const& circle,
        quad_color const& fill_color,
        quad_color const& border_color,
        float border_width,
        hi::border_side border_side) const noexcept
    {
        // clang-format off
        hilet circle_ =
            border_side == hi::border_side::inside ? circle - border_width * 0.5f :
            border_side == hi::border_side::outside ? circle + border_width * 0.5f :
            circle;
        // clang-format on

        hilet box = layout.to_window * make_rectangle(circle_);
        hilet corners = layout.to_window * make_corner_radii(circle_);
        return _draw_box(layout.window_clipping_rectangle(), box, fill_color, border_color, border_width, corners);
    }

    void draw_circle(
        widget_layout const& layout,
        aarectangle const& clipping_rectangle,
        hi::circle const& circle,
        quad_color const& fill_color,
        quad_color const& border_color,
        float border_width,
        hi::border_side border_side) const noexcept
    {
        // clang-format off
        hilet circle_ =
            border_side == hi::border_side::inside ? circle - border_width * 0.5f :
            border_side == hi::border_side::outside ? circle + border_width * 0.5f :
            circle;
        // clang-format on

        hilet box = layout.to_window * make_rectangle(circle_);
        hilet corners = layout.to_window * make_corner_radii(circle_);
        return _draw_box(
            layout.window_clipping_rectangle(clipping_rectangle), box, fill_color, border_color, border_width, corners);
    }

    /** Draw an image
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param image The image to show.
     * @return True when the image was drawn, false if the image is not ready yet.
     *         Widgets may want to request a redraw if the image is not ready.
     */
    [[nodiscard]] bool draw_image(widget_layout const& layout, quad const& box, paged_image& image) const noexcept
    {
        return _draw_image(layout.window_clipping_rectangle(), layout.to_window * box, image);
    }

    /** Draw an image
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param clipping_rectangle A more narrow clipping rectangle than supplied by layout.
     * @param box The four points of the box to draw.
     * @param image The image to show.
     * @return True when the image was drawn, false if the image is not ready yet.
     *         Widgets may want to request a redraw if the image is not ready.
     */
    [[nodiscard]] bool
    draw_image(widget_layout const& layout, aarectangle const& clipping_rectangle, quad const& box, paged_image& image)
        const noexcept
    {
        return _draw_image(layout.window_clipping_rectangle(clipping_rectangle), layout.to_window * box, image);
    }

    /** Draw a glyph.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The size and position of the glyph.
     * @param color The color that the glyph should be drawn in.
     * @param glyph The glyphs to draw.
     */
    void draw_glyph(widget_layout const& layout, quad const& box, quad_color const& color, glyph_ids const& glyph) const noexcept
    {
        return _draw_glyph(layout.window_clipping_rectangle(), layout.to_window * box, color, glyph);
    }

    /** Draw a glyph.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param clipping_rectangle A more narrow clipping rectangle than supplied by layout.
     * @param box The size and position of the glyph.
     * @param color The color that the glyph should be drawn in.
     * @param glyph The glyphs to draw.
     */
    void draw_glyph(
        widget_layout const& layout,
        aarectangle clipping_rectangle,
        quad const& box,
        quad_color const& color,
        glyph_ids const& glyph) const noexcept
    {
        return _draw_glyph(layout.window_clipping_rectangle(clipping_rectangle), layout.to_window * box, color, glyph);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param color Text-color overriding the colors from the shaped_text.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const& layout, matrix3 const& transform, quad_color const& color, shaped_text const& text)
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
        widget_layout const& layout,
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        quad_color const& color,
        shaped_text const& text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(clipping_rectangle), layout.to_window * transform, text, color);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const& layout, matrix3 const& transform, shaped_text const& text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(), layout.to_window * transform, text);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param color Text-color overriding the colors from the shaped_text.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const& layout, matrix3 const& transform, quad_color const& color, text_shaper const& text)
        const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(), layout.to_window * transform, text, color);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param color Text-color overriding the colors from the shaped_text.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const& layout, quad_color const& color, text_shaper const& text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(), layout.to_window, text, color);
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
        widget_layout const& layout,
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        quad_color const& color,
        text_shaper const& text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(clipping_rectangle), layout.to_window * transform, text, color);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param clipping_rectangle A more narrow clipping rectangle than supplied by layout.
     * @param color Text-color overriding the colors from the shaped_text.
     * @param text The shaped text to draw.
     */
    void draw_text(
        widget_layout const& layout,
        aarectangle const& clipping_rectangle,
        quad_color const& color,
        text_shaper const& text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(clipping_rectangle), layout.to_window, text, color);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const& layout, matrix3 const& transform, text_shaper const& text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(), layout.to_window * transform, text);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     */
    void draw_text(widget_layout const& layout, text_shaper const& text) const noexcept
    {
        return _draw_text(layout.window_clipping_rectangle(), layout.to_window, text);
    }

    /** Draw text-selection of shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     * @param selection The text selection.
     * @param color The color of the selection.
     */
    void
    draw_text_selection(widget_layout const& layout, text_shaper const& text, text_selection const& selection, hi::color color)
        const noexcept
    {
        return _draw_text_selection(layout.window_clipping_rectangle(), layout.to_window, text, selection, color);
    }

    /** Draw text cursors of shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     * @param cursor The position of the cursor.
     * @param primary_color The color of the primary cursor (the insertion cursor).
     * @param secondary_color The color of the secondary cursor (the append cursor).
     * @param overwrite_mode If true draw overwrite mode cursor; if false draw insertion mode cursors,
     * @param dead_character_mode If true draw the dead-character cursor. The dead_character_mode overrides all other cursors.
     */
    void draw_text_cursors(
        widget_layout const& layout,
        text_shaper const& text,
        text_cursor cursor,
        hi::color primary_color,
        hi::color secondary_color,
        bool overwrite_mode,
        bool dead_character_mode) const noexcept
    {
        return _draw_text_cursors(
            layout.window_clipping_rectangle(),
            layout.to_window,
            text,
            cursor,
            primary_color,
            secondary_color,
            overwrite_mode,
            dead_character_mode);
    }

    /** Make a hole in the user interface.
     *
     * This function makes a hole in the user-interface so that fragments written in the
     * swap-chain before the GUI is drawn will be visible.
     *
     * @param layout The layout of the widget.
     * @param box The box in local coordinates of the widget.
     */
    void make_hole(widget_layout const& layout, quad const& box) const noexcept
    {
        return _override_alpha(layout.window_clipping_rectangle(), layout.to_window * box, 0.0f);
    }

    [[nodiscard]] friend bool overlaps(draw_context const& context, widget_layout const& layout) noexcept
    {
        return overlaps(context.scissor_rectangle, layout.window_clipping_rectangle());
    }

private:
    vector_span<pipeline_box::vertex> *_box_vertices;
    vector_span<pipeline_image::vertex> *_image_vertices;
    vector_span<pipeline_SDF::vertex> *_sdf_vertices;
    vector_span<pipeline_alpha::vertex> *_alpha_vertices;

    void _override_alpha(aarectangle const& clipping_rectangle, quad box, float alpha) const noexcept;

    void _draw_box(
        aarectangle const& clipping_rectangle,
        quad box,
        quad_color const& fill_color,
        quad_color const& border_color,
        float border_width,
        hi::corner_radii corner_radius) const noexcept;

    void _draw_text(
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        shaped_text const& text,
        std::optional<quad_color> color = {}) const noexcept;

    void _draw_text(
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        std::optional<quad_color> color = {}) const noexcept;

    void _draw_text_selection(
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        text_selection const& selection,
        hi::color) const noexcept;

    void _draw_text_insertion_cursor_empty(
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        hi::color color) const noexcept;

    void _draw_text_insertion_cursor(
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        text_cursor cursor,
        hi::color color,
        bool show_flag) const noexcept;

    void _draw_text_overwrite_cursor(
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper::char_const_iterator it,
        hi::color color) const noexcept;

    void _draw_text_cursors(
        aarectangle const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        text_cursor cursor,
        hi::color primary_color,
        hi::color secondary_color,
        bool overwrite_mode,
        bool dead_character_mode) const noexcept;

    void _draw_glyph(aarectangle const& clipping_rectangle, quad const& box, quad_color const& color, glyph_ids const& glyph)
        const noexcept;

    [[nodiscard]] bool _draw_image(aarectangle const& clipping_rectangle, quad const& box, paged_image& image) const noexcept;
};

} // namespace hi::inline v1
