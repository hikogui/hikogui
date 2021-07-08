// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_flat_vertex.hpp"
#include "pipeline_box_vertex.hpp"
#include "pipeline_image_vertex.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/matrix.hpp"
#include "../geometry/corner_shapes.hpp"
#include "../geometry/identity.hpp"
#include "../color/color.hpp"
#include "../vspan.hpp"

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
    draw_context(draw_context const &rhs) noexcept = default;
    draw_context(draw_context &&rhs) noexcept = default;
    draw_context &operator=(draw_context const &rhs) noexcept = default;
    draw_context &operator=(draw_context &&rhs) noexcept = default;
    ~draw_context() = default;

    draw_context(
        gfx_device_vulkan &device,
        size_t frame_buffer_index,
        extent2 surface_size,
        aarectangle scissor_rectangle,
        vspan<pipeline_flat::vertex> &flatVertices,
        vspan<pipeline_box::vertex> &boxVertices,
        vspan<pipeline_image::vertex> &imageVertices,
        vspan<pipeline_SDF::vertex> &sdfVertices) noexcept;

    [[nodiscard]] draw_context
    make_child_context(matrix3 parent_to_local, matrix3 local_to_window, aarectangle clipping_rectangle) const noexcept;

    [[nodiscard]] size_t frame_buffer_index() const noexcept;

    [[nodiscard]] aarectangle scissor_rectangle() const noexcept;

    [[nodiscard]] aarectangle clipping_rectangle() const noexcept;

    void set_clipping_rectangle(aarectangle clipping_rectangle) noexcept;

    [[nodiscard]] matrix3 transform() const noexcept;

    gfx_device &device() const noexcept;

    /** Draw a polygon with four corners of one color.
     * This function will draw a polygon between the four given points.
     * This will use the current:
     *  - transform, to transform each point.
     *  - clippingRectangle
     *  - fill_color
     */
    void draw_filled_quad(point3 p1, point3 p2, point3 p3, point3 p4, color fill_color) const noexcept;

    /** Draw a rectangle of one color.
     * This function will draw the given rectangle.
     * This will use the current:
     *  - transform, to transform each corner of the rectangle.
     *  - clippingRectangle
     *  - fill_color
     */
    void draw_filled_quad(rectangle r, color fill_color) const noexcept;

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
        rectangle box,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept;

    void draw_box(rectangle box, color fill_color, color line_color, tt::corner_shapes corner_shapes) const noexcept;

    void draw_box(rectangle box, color fill_color, tt::corner_shapes corner_shapes) const noexcept;

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
        rectangle rectangle,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept;

    void draw_box_with_border_inside(rectangle rectangle, color fill_color, color line_color, tt::corner_shapes corner_shapes)
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
        rectangle rectangle,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept;

    void draw_box_with_border_outside(rectangle rectangle, color fill_color, color line_color, tt::corner_shapes corner_shapes)
        const noexcept;

    /** Draw an image
     * This function will draw an image.
     * This will use the current:
     *  - transform, to transform the image.
     *  - clippingRectangle
     */
    void draw_image(pipeline_image::image &image, matrix3 image_transform) const noexcept;

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
    void
    draw_text(shaped_text const &text, std::optional<color> text_color = {}, matrix3 transform = geo::identity{}) const noexcept;

    void draw_glyph(font_glyph_ids const &glyph, rectangle box, color text_color) const noexcept;

    [[nodiscard]] friend bool overlaps(draw_context const &context, aarectangle const &rectangle) noexcept
    {
        return overlaps(context._scissor_rectangle, rectangle);
    }

private:
    gfx_device_vulkan &_device;

    vspan<pipeline_flat::vertex> *_flat_vertices;
    vspan<pipeline_box::vertex> *_box_vertices;
    vspan<pipeline_image::vertex> *_image_vertices;
    vspan<pipeline_SDF::vertex> *_sdf_vertices;

    /** The frame buffer index of the image we are currently rendering.
     */
    size_t _frame_buffer_index;

    /** This is the rectangle of the window that is being redrawn.
     * The scissor rectangle, like drawing coordinates are relative to the widget.
     */
    aarectangle _scissor_rectangle;

    /** The clipping rectangle when drawing.
     * The clipping rectangle, like drawing coordinates are relative to the widget.
     */
    aarectangle _clipping_rectangle;

    /** Transform used on the given coordinates.
     * The z-axis translate is used for specifying the elevation
     * (inverse depth buffer) of the shape.
     */
    matrix3 _transform = geo::identity{};
};

} // namespace tt
