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
#include "../vspan.hpp"
#include "layout_context.hpp"

namespace tt {
class gfx_device;
class gfx_device_vulkan;
class shaped_text;
class font_glyph_ids;
namespace pipeline_image {
struct image;
}

/** Draw context for drawing using the TTauri shaders.
 */
class draw_context {
public:
    gfx_device_vulkan &device;

    /** The frame buffer index of the image we are currently rendering.
     */
    size_t frame_buffer_index;

    /** This is the rectangle of the window that is being redrawn.
     * The scissor rectangle, like drawing coordinates are relative to the widget.
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

    /** Draw an axis aligned box
     * This function will draw the given box.
     * This will use the current:
     *  - transform, to transform the opposite corner (rotation is not recommended).
     *  - clippingRectangle
     *  - fill_color
     *  - borderSize
     *  - border_color
     *  - shadowSize
     *  - cornerShapes
     */
    void draw_box(
        layout_context const &layout,
        rectangle box,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept;

    void
    draw_box(layout_context const &layout, rectangle box, color fill_color, color line_color, tt::corner_shapes corner_shapes)
        const noexcept;

    void draw_box(layout_context const &layout, rectangle box, color fill_color, tt::corner_shapes corner_shapes) const noexcept;

    void draw_box(layout_context const &layout, rectangle box, color fill_color) const noexcept;

    /** Draw an axis aligned box
     * This function will shrink to include the size of the border inside
     * the given rectangle. This will make the border be drawn sharply.
     *
     * This will also adjust rounded corners to the shrunk box.
     *
     * This function will draw the given box.
     * This will use the current:
     *  - transform, to transform the opposite corner (rotation is not recommended).
     *  - clipping_rectangle
     *  - fill_color
     *  - border_size
     *  - border_color
     *  - corner_shapes
     */
    void draw_box_with_border_inside(
        layout_context const &layout,
        rectangle rectangle,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept;

    void draw_box_with_border_inside(
        layout_context const &layout,
        rectangle rectangle,
        color fill_color,
        color line_color,
        tt::corner_shapes corner_shapes)
        const noexcept;

    /** Draw an axis aligned box
     * This function will expand to include the size of the border outside
     * the given rectangle. This will make the border be drawn sharply.
     *
     * This will also adjust rounded corners to the shrunk box.
     *
     * This function will draw the given box.
     * This will use the current:
     *  - transform, to transform the opposite corner (rotation is not recommended).
     *  - clippingRectangle
     *  - fill_color
     *  - borderSize
     *  - border_color
     *  - shadowSize
     *  - cornerShapes
     */
    void draw_box_with_border_outside(
        layout_context const &layout,
        rectangle rectangle,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept;

    void draw_box_with_border_outside(
        layout_context const &layout,
        rectangle rectangle,
        color fill_color,
        color line_color,
        tt::corner_shapes corner_shapes)
        const noexcept;

    /** Draw an image
     * This function will draw an image.
     * This will use the current:
     *  - transform, to transform the image.
     *  - clippingRectangle
     */
    void draw_image(layout_context const &layout, pipeline_image::image &image, matrix3 image_transform) const noexcept;

    /** Draw shaped text.
     * This function will draw the shaped text.
     * The SDF-image-atlas needs to be prepared ahead of time.
     * This will use the current:
     *  - transform, to transform the shaped-text's bounding box
     *  - clippingRectangle
     *
     * @param text The shaped text to draw.
     * @param useContextColor When true display the text in the context's color, if false use text style color
     */
    void draw_text(
        layout_context const &layout,
        shaped_text const &text,
        std::optional<color> text_color = {},
        matrix3 transform = geo::identity{}) const noexcept;

    /** Draw a glyph.
     *
     * @param glyph The glyphs to draw.
     * @param glyph_size The scale with which the glyph is being drawn.
     * @param box The size and position of the glyph. The size must be the size of the bounding box of the glyph
     *            multiplied by @a glyph_size.
     * @param text_color The color that the glyph should be drawn in.
     */
    void draw_glyph(layout_context const &layout, font_glyph_ids const &glyph, quad const &box, color text_color)
        const noexcept;

    [[nodiscard]] friend bool overlaps(draw_context const &context, layout_context const &layout) noexcept
    {
        return overlaps(context.scissor_rectangle, layout.redraw_rectangle);
    }

private:
    vspan<pipeline_box::vertex> *_box_vertices;
    vspan<pipeline_image::vertex> *_image_vertices;
    vspan<pipeline_SDF::vertex> *_sdf_vertices;

};

} // namespace tt
