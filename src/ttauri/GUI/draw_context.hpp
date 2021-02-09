// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "gui_device_vulkan.hpp"
#include "gui_window.hpp"
#include "theme.hpp"
#include "PipelineImage_Image.hpp"
#include "PipelineFlat_DeviceShared.hpp"
#include "PipelineBox_DeviceShared.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "PipelineSDF_DeviceShared.hpp"
#include "PipelineFlat_Vertex.hpp"
#include "PipelineBox_Vertex.hpp"
#include "PipelineImage_Vertex.hpp"
#include "PipelineSDF_Vertex.hpp"
#include "../numeric_array.hpp"
#include "../mat.hpp"
#include "../aarect.hpp"
#include "../vspan.hpp"
#include "../text/shaped_text.hpp"
#include <type_traits>

namespace tt {

/** Draw context for drawing using the TTauri shaders.
 */
class draw_context {
public:
    /// Foreground color.
    f32x4 color = f32x4::color(1.0, 1.0, 1.0, 1.0);

    /// Fill color.
    f32x4 fill_color = f32x4::color(0.0, 0.0, 0.0, 0.0);

    /// Size of lines.
    float line_width = 1.0;

    /** Shape of the corners of a box.
     * The vector holds information for each corner:
     *  - x: left-bottom
     *  - y: right-bottom
     *  - z: left-top
     *  - w: right-top
     *
     * The value means:
     *  - zero: Sharp corner
     *  - positive: Rounded corner of that radius
     *  - negative: Cur corner of that radius
     */
    f32x4 corner_shapes = f32x4{0.0, 0.0, 0.0, 0.0};

    /** The clipping rectangle when drawing.
     * The clipping rectangle is passes as-is to the pipelines and
     * is not modified by the transform.
     */
    aarect clipping_rectangle;

    /** Transform used on the given coordinates.
     * The z-axis translate is used for specifying the elevation
     * (inverse depth buffer) of the shape.
     */
    mat transform = mat::I();

    draw_context(
        gui_window &window,
        aarect scissor_rectangle,
        vspan<PipelineFlat::Vertex> &flatVertices,
        vspan<PipelineBox::Vertex> &boxVertices,
        vspan<PipelineImage::Vertex> &imageVertices,
        vspan<PipelineSDF::Vertex> &sdfVertices) noexcept :
        _window(&window),
        _scissor_rectangle(scissor_rectangle),
        _flat_vertices(&flatVertices),
        _box_vertices(&boxVertices),
        _image_vertices(&imageVertices),
        _sdf_vertices(&sdfVertices),
        color(0.0, 1.0, 0.0, 1.0),
        fill_color(1.0, 1.0, 0.0, 1.0),
        line_width(theme::global->borderWidth),
        corner_shapes(),
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
    void draw_filled_quad(f32x4 p1, f32x4 p2, f32x4 p3, f32x4 p4) const noexcept
    {
        tt_axiom(_flat_vertices != nullptr);
        _flat_vertices->emplace_back(transform * p1, clipping_rectangle, fill_color);
        _flat_vertices->emplace_back(transform * p2, clipping_rectangle, fill_color);
        _flat_vertices->emplace_back(transform * p3, clipping_rectangle, fill_color);
        _flat_vertices->emplace_back(transform * p4, clipping_rectangle, fill_color);
    }

    /** Draw a rectangle of one color.
     * This function will draw the given rectangle.
     * This will use the current:
     *  - transform, to transform each corner of the rectangle.
     *  - clippingRectangle
     *  - fillColor
     */
    void draw_filled_quad(aarect r) const noexcept
    {
        draw_filled_quad(r.corner<0>(), r.corner<1>(), r.corner<2>(), r.corner<3>());
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
    void draw_box(aarect box) const noexcept
    {
        tt_axiom(_box_vertices != nullptr);

        PipelineBox::DeviceShared::placeVertices(
            *_box_vertices, transform * box, fill_color, line_width, color, corner_shapes, clipping_rectangle);
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
    void draw_box_with_border_inside(aarect rectangle) const noexcept
    {
        tt_axiom(_box_vertices != nullptr);

        ttlet shrink_value = line_width * 0.5f;

        ttlet new_rectangle = shrink(rectangle, shrink_value);

        ttlet new_corner_shapes =
            f32x4{std::max(0.0f, corner_shapes.x() - shrink_value),
                std::max(0.0f, corner_shapes.y() - shrink_value),
                std::max(0.0f, corner_shapes.z() - shrink_value),
                std::max(0.0f, corner_shapes.w() - shrink_value)};

        PipelineBox::DeviceShared::placeVertices(
            *_box_vertices, transform * new_rectangle, fill_color, line_width, color, new_corner_shapes, clipping_rectangle);
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
    void draw_box_with_border_outside(aarect rectangle) const noexcept
    {
        tt_axiom(_box_vertices != nullptr);

        ttlet shrink_value = line_width * 0.5f;

        ttlet new_rectangle = expand(rectangle, shrink_value);

        ttlet new_corner_shapes =
            f32x4{std::max(0.0f, corner_shapes.x() - shrink_value),
                std::max(0.0f, corner_shapes.y() - shrink_value),
                std::max(0.0f, corner_shapes.z() - shrink_value),
                std::max(0.0f, corner_shapes.w() - shrink_value)};

        PipelineBox::DeviceShared::placeVertices(
            *_box_vertices, transform * new_rectangle, fill_color, line_width, color, new_corner_shapes, clipping_rectangle);
    }

    /** Draw an image
     * This function will draw an image.
     * This will use the current:
     *  - transform, to transform the image.
     *  - clippingRectangle
     */
    void draw_image(PipelineImage::Image &image) const noexcept
    {
        tt_axiom(_image_vertices != nullptr);

        image.placeVertices(*_image_vertices, transform, clipping_rectangle);
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
    void draw_text(shaped_text const &text, bool useContextColor = false) const noexcept
    {
        tt_axiom(_sdf_vertices != nullptr);

        if (useContextColor) {
            narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->placeVertices(
                *_sdf_vertices, text, transform, clipping_rectangle, color);
        } else {
            narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->placeVertices(
                *_sdf_vertices, text, transform, clipping_rectangle);
        }
    }

    void draw_glyph(font_glyph_ids const &glyph, aarect box) const noexcept
    {
        tt_axiom(_sdf_vertices != nullptr);

        narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->placeVertices(
            *_sdf_vertices, glyph, transform * box, color, clipping_rectangle);
    }

    [[nodiscard]] friend bool overlaps(draw_context const &context, aarect const &rectangle) noexcept
    {
        return overlaps(context._scissor_rectangle, rectangle);
    }

private:
    gui_window *_window;
public:
    aarect _scissor_rectangle;
private:
    vspan<PipelineFlat::Vertex> *_flat_vertices;
    vspan<PipelineBox::Vertex> *_box_vertices;
    vspan<PipelineImage::Vertex> *_image_vertices;
    vspan<PipelineSDF::Vertex> *_sdf_vertices;
};

} // namespace tt
