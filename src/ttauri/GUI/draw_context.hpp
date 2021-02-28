// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "gui_device_vulkan.hpp"
#include "gui_window.hpp"
#include "theme.hpp"
#include "pipeline_image_image.hpp"
#include "pipeline_flat_device_shared.hpp"
#include "pipeline_box_device_shared.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "pipeline_flat_vertex.hpp"
#include "pipeline_box_vertex.hpp"
#include "pipeline_image_vertex.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../vspan.hpp"
#include "../text/shaped_text.hpp"
#include "../color/color.hpp"
#include "../geometry/corner_shapes.hpp"
#include <type_traits>

namespace tt {

/** Draw context for drawing using the TTauri shaders.
 */
class draw_context {
public:
    /** This is the rectangle of the window that is being redrawn.
     * The scissor rectangle, like drawing coordinates are relative to the widget.
     */
    aarect scissor_rectangle;

    /** The clipping rectangle when drawing.
     * The clipping rectangle, like drawing coordinates are relative to the widget.
     */
    aarect clipping_rectangle;

    /** Transform used on the given coordinates.
     * The z-axis translate is used for specifying the elevation
     * (inverse depth buffer) of the shape.
     */
    matrix3 transform = geo::identity{};

    draw_context(
        gui_window &window,
        aarect scissor_rectangle,
        vspan<pipeline_flat::vertex> &flatVertices,
        vspan<pipeline_box::vertex> &boxVertices,
        vspan<pipeline_image::vertex> &imageVertices,
        vspan<pipeline_SDF::vertex> &sdfVertices) noexcept :
        _window(&window),
        scissor_rectangle(scissor_rectangle),
        _flat_vertices(&flatVertices),
        _box_vertices(&boxVertices),
        _image_vertices(&imageVertices),
        _sdf_vertices(&sdfVertices),
        clipping_rectangle(static_cast<f32x4>(window.extent))
    {
        _flat_vertices->clear();
        _box_vertices->clear();
        _image_vertices->clear();
        _sdf_vertices->clear();
    }

    draw_context(draw_context const &rhs) noexcept = default;
    draw_context(draw_context &&rhs) noexcept = default;
    draw_context &operator=(draw_context const &rhs) noexcept = default;
    draw_context &operator=(draw_context &&rhs) noexcept = default;
    ~draw_context() = default;

    gui_window &window() const noexcept
    {
        tt_axiom(_window);
        return *_window;
    }

    gui_device &device() const noexcept
    {
        auto device = window().device();
        tt_axiom(device);
        return *device;
    }

    /** Draw a polygon with four corners of one color.
     * This function will draw a polygon between the four given points.
     * This will use the current:
     *  - transform, to transform each point.
     *  - clippingRectangle
     *  - fillColor
     */
    void draw_filled_quad(point3 p1, point3 p2, point3 p3, point3 p4, color fill_color) const noexcept
    {
        tt_axiom(_flat_vertices != nullptr);
        _flat_vertices->emplace_back(aarect{transform * clipping_rectangle}, transform * p1, fill_color);
        _flat_vertices->emplace_back(aarect{transform * clipping_rectangle}, transform * p2, fill_color);
        _flat_vertices->emplace_back(aarect{transform * clipping_rectangle}, transform * p3, fill_color);
        _flat_vertices->emplace_back(aarect{transform * clipping_rectangle}, transform * p4, fill_color);
    }

    /** Draw a rectangle of one color.
     * This function will draw the given rectangle.
     * This will use the current:
     *  - transform, to transform each corner of the rectangle.
     *  - clippingRectangle
     *  - fillColor
     */
    void draw_filled_quad(rect r, color fill_color) const noexcept
    {
        draw_filled_quad(get<0>(r), get<1>(r), get<2>(r), get<3>(r), fill_color);
    }

    /** Draw a rectangle of one color.
     * This function will draw the given rectangle.
     * This will use the current:
     *  - transform, to transform each corner of the rectangle.
     *  - clippingRectangle
     *  - fillColor
     */
    void draw_filled_quad(aarect r, color fill_color) const noexcept
    {
        draw_filled_quad(rect{r}, fill_color);
    }

    /** Draw an axis aligned box
     * This function will draw the given box.
     * This will use the current:
     *  - transform, to transform the opposite corner (rotation is not recommended).
     *  - clippingRectangle
     *  - fillColor
     *  - borderSize
     *  - borderColor
     *  - shadowSize
     *  - cornerShapes
     */
    void draw_box(
        aarect box,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept
    {
        tt_axiom(_box_vertices != nullptr);

        pipeline_box::device_shared::place_vertices(
            *_box_vertices, clipping_rectangle, transform * box, fill_color, line_color, line_width, corner_shapes);
    }

    void draw_box(
        aarect box,
        color fill_color,
        color line_color,
        tt::corner_shapes corner_shapes) const noexcept
    {
        draw_box(box, fill_color, line_color, 1.0, corner_shapes);
    }

    void draw_box(aarect box, color fill_color, tt::corner_shapes corner_shapes) const noexcept
    {
        draw_box(box, fill_color, fill_color, 0.0, corner_shapes);
    }

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
        aarect rectangle,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept
    {
        tt_axiom(_box_vertices != nullptr);

        ttlet shrink_value = line_width * 0.5f;

        ttlet new_rectangle = shrink(rectangle, shrink_value);

        ttlet new_corner_shapes = corner_shapes - shrink_value;

        pipeline_box::device_shared::place_vertices(
            *_box_vertices, clipping_rectangle, transform * new_rectangle, fill_color, line_color, line_width, new_corner_shapes);
    }

    void draw_box_with_border_inside(
        aarect rectangle,
        color fill_color,
        color line_color,
        tt::corner_shapes corner_shapes) const noexcept
    {
        draw_box_with_border_inside(rectangle, fill_color, line_color, 1.0, corner_shapes);
    }

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
     *  - fillColor
     *  - borderSize
     *  - borderColor
     *  - shadowSize
     *  - cornerShapes
     */
    void draw_box_with_border_outside(
        aarect rectangle,
        color fill_color,
        color line_color,
        float line_width = 1.0,
        tt::corner_shapes corner_shapes = tt::corner_shapes{}) const noexcept
    {
        tt_axiom(_box_vertices != nullptr);

        ttlet expand_value = line_width * 0.5f;

        ttlet new_rectangle = expand(rectangle, expand_value);

        ttlet new_corner_shapes = corner_shapes + expand_value;

        pipeline_box::device_shared::place_vertices(
            *_box_vertices, clipping_rectangle, transform * new_rectangle, fill_color, line_color, line_width, new_corner_shapes);
    }

    void draw_box_with_border_outside(aarect rectangle, color fill_color, color line_color, tt::corner_shapes corner_shapes)
        const noexcept
    {
        draw_box_with_border_outside(rectangle, fill_color, line_color, 1.0, corner_shapes);
    }

    /** Draw an image
     * This function will draw an image.
     * This will use the current:
     *  - transform, to transform the image.
     *  - clippingRectangle
     */
    void draw_image(pipeline_image::Image &image) const noexcept
    {
        tt_axiom(_image_vertices != nullptr);

        image.place_vertices(*_image_vertices, aarect{transform * clipping_rectangle}, transform);
    }

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
    void draw_text(shaped_text const &text, std::optional<color> text_color = {}) const noexcept
    {
        tt_axiom(_sdf_vertices != nullptr);

        if (text_color) {
            narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->place_vertices(
                *_sdf_vertices, aarect{transform * clipping_rectangle}, transform, text, *text_color);
        } else {
            narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->place_vertices(
                *_sdf_vertices, aarect{transform * clipping_rectangle}, transform, text);
        }
    }

    void draw_glyph(font_glyph_ids const &glyph, aarect box, color text_color) const noexcept
    {
        tt_axiom(_sdf_vertices != nullptr);

        narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->place_vertices(
            *_sdf_vertices, aarect{transform * clipping_rectangle}, transform * box, glyph, text_color);
    }

    [[nodiscard]] friend bool overlaps(draw_context const &context, aarect const &rectangle) noexcept
    {
        return overlaps(context.scissor_rectangle, rectangle);
    }

private:
    gui_window *_window;

    vspan<pipeline_flat::vertex> *_flat_vertices;
    vspan<pipeline_box::vertex> *_box_vertices;
    vspan<pipeline_image::vertex> *_image_vertices;
    vspan<pipeline_SDF::vertex> *_sdf_vertices;
};

} // namespace tt
