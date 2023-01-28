// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_box_vertex.hpp"
#include "pipeline_image_vertex.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "pipeline_alpha_vertex.hpp"
#include "subpixel_orientation.hpp"
#include "../geometry/module.hpp"
#include "../unicode/unicode_bidi_class.hpp"
#include "../text/text_cursor.hpp"
#include "../text/text_selection.hpp"
#include "../text/text_shaper.hpp"
#include "../color/module.hpp"
#include "../widgets/widget_layout.hpp"
#include "../vector_span.hpp"
#include "../utility/module.hpp"

namespace hi { inline namespace v1 {
class gfx_device;
class gfx_device_vulkan;
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

template<typename Context>
concept draw_attribute = std::same_as<Context, quad_color> or std::same_as<Context, color> or
    std::same_as<Context, border_side> or std::same_as<Context, line_end_cap> or std::same_as<Context, corner_radii> or
    std::same_as<Context, aarectanglei> or std::same_as<Context, float> or std::same_as<Context, int>;

/** The draw attributes used to draw shaped into the draw context.
 */
struct draw_attributes {
    unsigned char num_colors = 0;
    unsigned char num_line_caps = 0;

    /** The fill color used for the color of a box inside the border.
     *
     * This is also used as the line-color when drawing lines. Or the
     * color of the text. And the color for the primary cursor.
     */
    quad_color fill_color = {};

    /** The line color used for the color of the border of the box.
     *
     * This is also used as the color for the secondary cursor.
     */
    quad_color line_color = {};

    /** The width of a line, or the width of a border.
     */
    float line_width = 0.0f;

    /** The side on which side of the edge of a shape the border should be drawn.
     */
    hi::border_side border_side = hi::border_side::on;

    /** The radii of each corner of a quad.
     */
    hi::corner_radii corner_radius = {};

    /** The rectangle used the clip the shape when drawing.
     *
     * This rectangle is used for limiting drawing outside of a widget's rectangle.
     * But it may also be used to cut shapes for special effects.
     */
    aarectanglei clipping_rectangle = aarectanglei::large();

    /** The shape of the beginning of a line.
     */
    line_end_cap begin_line_cap = line_end_cap::flat;

    /** The shape of the beginning of a line.
     */
    line_end_cap end_line_cap = line_end_cap::flat;

    constexpr draw_attributes(draw_attributes const&) noexcept = default;
    constexpr draw_attributes(draw_attributes&&) noexcept = default;
    constexpr draw_attributes& operator=(draw_attributes const&) noexcept = default;
    constexpr draw_attributes& operator=(draw_attributes&&) noexcept = default;
    constexpr draw_attributes() noexcept = default;

    /** Construct the draw attributes based on types and order of the attributes.
     *
     * The following order of attributes is maintained:
     *  - By default the `fill_color` and `line_color` are transparent.
     *  - The first `hi::color` or `hi::quad_color` is used for the `fill_color`.
     *  - The second `hi::color` or `hi::quad_color` is used for the `line_color`.
     *  - By default the `begin_line_cap` and `end_line_cap` are set to flat.
     *  - The first `hi::line_end_cap` is used for both the `begin_line_cap` and `end_line_cap`.
     *  - The second `hi::line_end_cap` is used to override the `end_line_cap`.
     *  - By default the `border_side` is set to `border_side::on`
     *  - A `hi::border_side` argument is used to set the `border_side`.
     *  - By default the `corner_radius` are set to (0, 0, 0, 0).
     *  - A `hi::corner_radii` argument is used to set the `corner_radius`.
     *  - By default the `clipping_rectangle` is set to a rectangle encompassing the whole window.
     *  - A `hi::aarectangle` argument is used to set the `clipping_rectangle`.
     *  - By default the `line_width` is set to 0.
     *  - A `float` or `int` argument is used to set the `line_width`.
     *
     * @param args The attributes to be set.
     */
    template<draw_attribute... Args>
    constexpr draw_attributes(Args const&...args) noexcept
    {
        add(args...);
    }

    constexpr void add() noexcept {}

    template<draw_attribute T>
    constexpr void add(T const& attribute) noexcept
    {
        if constexpr (std::is_same_v<T, quad_color>) {
            if (num_colors++ == 0) {
                fill_color = attribute;
            } else {
                line_color = attribute;
            }
            hi_axiom(num_colors <= 2);

        } else if constexpr (std::is_same_v<T, color>) {
            if (num_colors++ == 0) {
                fill_color = quad_color{attribute};
            } else {
                line_color = quad_color{attribute};
            }
            hi_axiom(num_colors <= 2);

        } else if constexpr (std::is_same_v<T, line_end_cap>) {
            if (num_line_caps++ == 0) {
                begin_line_cap = attribute;
                end_line_cap = attribute;
            } else {
                end_line_cap = attribute;
            }
            hi_axiom(num_line_caps <= 2);

        } else if constexpr (std::is_same_v<T, hi::border_side>) {
            border_side = attribute;
#ifndef NDEBUG
            hi_assert(not _has_border_side);
            _has_border_side = true;
#endif

        } else if constexpr (std::is_same_v<T, corner_radii>) {
            corner_radius = attribute;
#ifndef NDEBUG
            hi_assert(not _has_corner_radii);
            _has_corner_radii = true;
#endif

        } else if constexpr (std::is_same_v<T, aarectanglei>) {
            clipping_rectangle = attribute;
#ifndef NDEBUG
            hi_assert(not _has_clipping_rectangle);
            _has_clipping_rectangle = true;
#endif

        } else if constexpr (std::is_same_v<T, float> or std::is_same_v<T, int>) {
            line_width = narrow_cast<float>(attribute);
#ifndef NDEBUG
            hi_assert(not _has_line_width);
            _has_line_width = true;
#endif
        } else {
            hi_static_no_default();
        }
    }

    template<draw_attribute First, draw_attribute Second, draw_attribute... Rest>
    constexpr void add(First const& first, Second const& second, Rest const&...rest) noexcept
    {
        add(first);
        add(second, rest...);
    }

private:
#ifndef NDEBUG
    bool _has_border_side = false;
    bool _has_corner_radii = false;
    bool _has_clipping_rectangle = false;
    bool _has_line_width = false;
#endif
};

template<typename Context>
concept draw_quad_shape = std::same_as<Context, quad> or std::same_as<Context, rectangle> or std::same_as<Context, aarectangle> or
    std::same_as<Context, aarectanglei>;

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
    aarectanglei scissor_rectangle;

    /** The background color to clear the window with.
     */
    color background_color;

    /** The subpixel orientation for rendering glyphs.
     */
    hi::subpixel_orientation subpixel_orientation;

    /** Window is active.
     */
    bool active;

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

    /** Draw a box.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param attributes The drawing attributes to use.
     */
    void draw_box(widget_layout const& layout, quad const& box, draw_attributes const& attributes) const noexcept
    {
        return _draw_box(
            layout.clipping_rectangle_on_window(attributes.clipping_rectangle), layout.to_window3() * box, attributes);
    }

    /** Draw a box.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param shape The shape of the box.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_quad_shape Shape, draw_attribute... Attributes>
    void draw_box(widget_layout const& layout, Shape const& shape, Attributes const&...attributes) const noexcept
    {
        return draw_box(layout, make_quad(shape), draw_attributes{attributes...});
    }

    /** Draw a line.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param line The line segment to draw.
     * @param attributes The drawing attributes to use.
     */
    void draw_line(widget_layout const& layout, line_segment const& line, draw_attributes const& attributes) const noexcept
    {
        hilet box = make_rectangle(line, attributes.line_width, attributes.begin_line_cap, attributes.end_line_cap);

        auto box_attributes = attributes;
        box_attributes.line_width = 0.0f;
        box_attributes.corner_radius =
            make_corner_radii(attributes.line_width, attributes.begin_line_cap, attributes.end_line_cap);
        return draw_box(layout, box, box_attributes);
    }

    /** Draw a line.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param line The line segment to draw.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_attribute... Attributes>
    void draw_line(widget_layout const& layout, line_segment const& line, Attributes const&...attributes) const noexcept
    {
        return draw_line(layout, line, draw_attributes{attributes...});
    }

    /** Draw a circle.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param circle The circle to draw.
     * @param attributes The drawing attributes to use.
     */
    void draw_circle(widget_layout const& layout, hi::circle const& circle, draw_attributes const& attributes) const noexcept
    {
        auto box_attributes = attributes;
        box_attributes.corner_radius = make_corner_radii(circle);
        return draw_box(layout, make_rectangle(circle), box_attributes);
    }

    /** Draw a circle.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param circle The circle to draw.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_attribute... Attributes>
    void draw_circle(widget_layout const& layout, hi::circle const& circle, Attributes const&...attributes) const noexcept
    {
        return draw_circle(layout, circle, draw_attributes{attributes...});
    }

    /** Draw an image
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param image The image to show.
     * @param attributes The drawing attributes to use.
     * @return True when the image was drawn, false if the image is not ready yet.
     *         Widgets may want to request a redraw if the image is not ready.
     */
    [[nodiscard]] bool
    draw_image(widget_layout const& layout, quad const& box, paged_image& image, draw_attributes const& attributes) const noexcept
    {
        return _draw_image(layout.clipping_rectangle_on_window(attributes.clipping_rectangle), layout.to_window3() * box, image);
    }

    /** Draw an image
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The four points of the box to draw.
     * @param image The image to show.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     * @return True when the image was drawn, false if the image is not ready yet.
     *         Widgets may want to request a redraw if the image is not ready.
     */
    template<draw_attribute... Attributes>
    [[nodiscard]] bool
    draw_image(widget_layout const& layout, draw_quad_shape auto const& box, paged_image& image, Attributes const&...attributes)
        const noexcept
    {
        return draw_image(layout, make_quad(box), image, draw_attributes{attributes...});
    }

    /** Draw a glyph.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The size and position of the glyph.
     * @param glyph The glyphs to draw.
     * @param attributes The drawing attributes to use.
     */
    void draw_glyph(widget_layout const& layout, quad const& box, glyph_ids const& glyph, draw_attributes const& attributes)
        const noexcept
    {
        return _draw_glyph(
            layout.clipping_rectangle_on_window(attributes.clipping_rectangle), layout.to_window3() * box, glyph, attributes);
    }

    /** Draw a glyph.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param box The size and position of the glyph.
     * @param glyph The glyphs to draw.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_quad_shape Shape, draw_attribute... Attributes>
    void draw_glyph(widget_layout const& layout, Shape const& box, glyph_ids const& glyph, Attributes const&...attributes)
        const noexcept
    {
        return draw_glyph(layout, make_quad(box), glyph, draw_attributes{attributes...});
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param text The shaped text to draw.
     * @param attributes The drawing attributes to use.
     */
    void
    draw_text(widget_layout const& layout, matrix3 const& transform, text_shaper const& text, draw_attributes const& attributes)
        const noexcept
    {
        return _draw_text(
            layout.clipping_rectangle_on_window(attributes.clipping_rectangle),
            layout.to_window3() * transform,
            text,
            attributes);
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param transform How to transform the shaped text relative to layout.
     * @param text The shaped text to draw.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_attribute... Attributes>
    void draw_text(widget_layout const& layout, matrix3 const& transform, text_shaper const& text, Attributes const&...attributes)
        const noexcept
    {
        return draw_text(layout, transform, text, draw_attributes{attributes...});
    }

    /** Draw shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_attribute... Attributes>
    void draw_text(widget_layout const& layout, text_shaper const& text, Attributes const&...attributes) const noexcept
    {
        return draw_text(layout, geo::identity{}, text, draw_attributes{attributes...});
    }

    /** Draw text-selection of shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     * @param selection The text selection.
     * @param attributes The drawing attributes to use.
     */
    void draw_text_selection(
        widget_layout const& layout,
        text_shaper const& text,
        text_selection const& selection,
        draw_attributes const& attributes) const noexcept
    {
        return _draw_text_selection(
            layout.clipping_rectangle_on_window(attributes.clipping_rectangle), layout.to_window3(), text, selection, attributes);
    }

    /** Draw text-selection of shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     * @param selection The text selection.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_attribute... Attributes>
    void draw_text_selection(
        widget_layout const& layout,
        text_shaper const& text,
        text_selection const& selection,
        Attributes const&...attributes) const noexcept
    {
        return draw_text_selection(layout, text, selection, draw_attributes{attributes...});
    }

    /** Draw text cursors of shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     * @param cursor The position of the cursor.
     * @param overwrite_mode If true draw overwrite mode cursor; if false draw insertion mode cursors,
     * @param dead_character_mode If true draw the dead-character cursor. The dead_character_mode overrides all other cursors.
     * @param attributes The drawing attributes to use.
     */
    void draw_text_cursors(
        widget_layout const& layout,
        text_shaper const& text,
        text_cursor cursor,
        bool overwrite_mode,
        bool dead_character_mode,
        draw_attributes const& attributes) const noexcept
    {
        return _draw_text_cursors(
            layout.clipping_rectangle_on_window(attributes.clipping_rectangle),
            layout.to_window3(),
            text,
            cursor,
            overwrite_mode,
            dead_character_mode,
            attributes);
    }

    /** Draw text cursors of shaped text.
     *
     * @param layout The layout to use, specifically the to_window transformation matrix and the clipping rectangle.
     * @param text The shaped text to draw.
     * @param cursor The position of the cursor.
     * @param overwrite_mode If true draw overwrite mode cursor; if false draw insertion mode cursors,
     * @param dead_character_mode If true draw the dead-character cursor. The dead_character_mode overrides all other cursors.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_attribute... Attributes>
    void draw_text_cursors(
        widget_layout const& layout,
        text_shaper const& text,
        text_cursor cursor,
        bool overwrite_mode,
        bool dead_character_mode,
        Attributes const&...attributes) const noexcept
    {
        return draw_text_cursors(layout, text, cursor, overwrite_mode, dead_character_mode, draw_attributes{attributes...});
    }

    /** Make a hole in the user interface.
     *
     * This function makes a hole in the user-interface so that fragments written in the
     * swap-chain before the GUI is drawn will be visible.
     *
     * @param layout The layout of the widget.
     * @param box The box in local coordinates of the widget.
     * @param attributes The drawing attributes to use.
     */
    void draw_hole(widget_layout const& layout, quad const& box, draw_attributes const& attributes) const noexcept
    {
        return _override_alpha(
            layout.clipping_rectangle_on_window(attributes.clipping_rectangle), layout.to_window3() * box, attributes);
    }

    /** Make a hole in the user interface.
     *
     * This function makes a hole in the user-interface so that fragments written in the
     * swap-chain before the GUI is drawn will be visible.
     *
     * @param layout The layout of the widget.
     * @param box The box in local coordinates of the widget.
     * @param attributes The drawing attributes to use, see: `draw_attributes::draw_attributes()`.
     */
    template<draw_quad_shape Shape, draw_attribute... Attributes>
    void draw_hole(widget_layout const& layout, Shape const& box, Attributes const&...attributes) const noexcept
    {
        return draw_hole(layout, make_quad(box), draw_attributes{attributes...});
    }

    /** Checks if a widget's layout overlaps with the part of the window that is being drawn.
     *
     * @param context The draw context which contains the scissor rectangle.
     * @param layout The layout of a widget which contains the rectangle where the widget is located
     *               on the window
     * @return True if the widget needs to draw into the context.
     */
    [[nodiscard]] friend bool overlaps(draw_context const& context, widget_layout const& layout) noexcept
    {
        return overlaps(context.scissor_rectangle, layout.clipping_rectangle_on_window());
    }

private:
    vector_span<pipeline_box::vertex> *_box_vertices;
    vector_span<pipeline_image::vertex> *_image_vertices;
    vector_span<pipeline_SDF::vertex> *_sdf_vertices;
    vector_span<pipeline_alpha::vertex> *_alpha_vertices;

    template<draw_quad_shape Shape>
    [[nodiscard]] constexpr static quad make_quad(Shape const& shape) noexcept
    {
        if constexpr (std::is_same_v<Shape, aarectanglei>) {
            return narrow_cast<aarectangle>(shape);
        } else {
            return shape;
        }
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

    [[nodiscard]] constexpr static rectangle make_rectangle(hi::circle const& circle) noexcept
    {
        hilet circle_ = f32x4{circle};
        hilet origin = point3{circle_.xyz1() - circle_.ww00()};
        hilet right = vector3{circle_.w000() * 2.0f};
        hilet up = vector3{circle_._0w00() * 2.0f};
        return rectangle{origin, right, up};
    }

    [[nodiscard]] constexpr static corner_radii make_corner_radii(float width, line_end_cap c1, line_end_cap c2) noexcept
    {
        auto r = f32x4::broadcast(width * 0.5f);

        if (c1 == line_end_cap::flat) {
            r = set_zero<0b0101>(r);
        }
        if (c2 == line_end_cap::flat) {
            r = set_zero<0b1010>(r);
        }

        return corner_radii{r};
    }

    [[nodiscard]] constexpr static corner_radii make_corner_radii(hi::circle const& circle) noexcept
    {
        return corner_radii{f32x4{circle}.wwww()};
    }

    void _override_alpha(aarectanglei const& clipping_rectangle, quad box, draw_attributes const& attributes) const noexcept;

    void _draw_box(aarectanglei const& clipping_rectangle, quad box, draw_attributes const& attributes) const noexcept;

    void _draw_text(
        aarectanglei const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        draw_attributes const& attributes) const noexcept;

    void _draw_text_selection(
        aarectanglei const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        text_selection const& selection,
        draw_attributes const& attributes) const noexcept;

    void _draw_text_insertion_cursor_empty(
        aarectanglei const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        draw_attributes const& attributes) const noexcept;

    void _draw_text_insertion_cursor(
        aarectanglei const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        text_cursor cursor,
        bool show_flag,
        draw_attributes const& attributes) const noexcept;

    void _draw_text_overwrite_cursor(
        aarectanglei const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper::char_const_iterator it,
        draw_attributes const& attributes) const noexcept;

    void _draw_text_cursors(
        aarectanglei const& clipping_rectangle,
        matrix3 const& transform,
        text_shaper const& text,
        text_cursor cursor,
        bool overwrite_mode,
        bool dead_character_mode,
        draw_attributes const& attributes) const noexcept;

    void _draw_glyph(
        aarectanglei const& clipping_rectangle,
        quad const& box,
        glyph_ids const& glyph,
        draw_attributes const& attributes) const noexcept;

    [[nodiscard]] bool _draw_image(aarectanglei const& clipping_rectangle, quad const& box, paged_image const& image) const noexcept;
};

}} // namespace hi::v1
