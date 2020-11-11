
// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "gui_device_vulkan.hpp"
#include "Window_base.hpp"
#include "Theme.hpp"
#include "PipelineImage_Image.hpp"
#include "PipelineFlat_DeviceShared.hpp"
#include "PipelineBox_DeviceShared.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "PipelineSDF_DeviceShared.hpp"
#include "PipelineFlat_Vertex.hpp"
#include "PipelineBox_Vertex.hpp"
#include "PipelineImage_Vertex.hpp"
#include "PipelineSDF_Vertex.hpp"
#include "../vec.hpp"
#include "../mat.hpp"
#include "../aarect.hpp"
#include "../vspan.hpp"
#include "../text/ShapedText.hpp"
#include <type_traits>

namespace tt {

/** Draw context for drawing using the TTauri shaders.
 */
class DrawContext {
private:
    Window_base *_window;
    vspan<PipelineFlat::Vertex> *flatVertices;
    vspan<PipelineBox::Vertex> *boxVertices;
    vspan<PipelineImage::Vertex> *imageVertices;
    vspan<PipelineSDF::Vertex> *sdfVertices;

public:
    /// Foreground color.
    vec color = vec::color(1.0, 1.0, 1.0, 1.0);

    /// Fill color.
    vec fillColor = vec::color(0.0, 0.0, 0.0, 0.0);

    /// Size of lines.
    float lineWidth = 1.0;

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
    vec cornerShapes = vec{0.0, 0.0, 0.0, 0.0};

    /** The clipping rectangle when drawing.
     * The clipping rectangle is passes as-is to the pipelines and
     * is not modified by the transform.
     */
    aarect clippingRectangle;

    /** Transform used on the given coordinates.
     * The z-axis translate is used for specifying the elevation
     * (inverse depth buffer) of the shape.
     */
    mat transform = mat::I();

    DrawContext(
        Window_base &window,
        vspan<PipelineFlat::Vertex> &flatVertices,
        vspan<PipelineBox::Vertex> &boxVertices,
        vspan<PipelineImage::Vertex> &imageVertices,
        vspan<PipelineSDF::Vertex> &sdfVertices) noexcept :
        _window(&window),
        flatVertices(&flatVertices),
        boxVertices(&boxVertices),
        imageVertices(&imageVertices),
        sdfVertices(&sdfVertices),
        color(0.0, 1.0, 0.0, 1.0),
        fillColor(1.0, 1.0, 0.0, 1.0),
        lineWidth(Theme::borderWidth),
        cornerShapes(),
        clippingRectangle(static_cast<vec>(window.currentWindowExtent))
    {
        flatVertices.clear();
        boxVertices.clear();
        imageVertices.clear();
        sdfVertices.clear();
    }

    DrawContext(DrawContext const &rhs) noexcept = default;
    DrawContext(DrawContext &&rhs) noexcept = default;
    DrawContext &operator=(DrawContext const &rhs) noexcept = default;
    DrawContext &operator=(DrawContext &&rhs) noexcept = default;
    ~DrawContext() = default;

    Window_base &window() const noexcept
    {
        tt_assume(_window);
        return *_window;
    }

    gui_device &device() const noexcept
    {
        auto device = window().device();
        tt_assume(device);
        return *device;
    }

    /** Draw a polygon with four corners of one color.
     * This function will draw a polygon between the four given points.
     * This will use the current:
     *  - transform, to transform each point.
     *  - clippingRectangle
     *  - fillColor
     */
    void drawFilledQuad(vec p1, vec p2, vec p3, vec p4) const noexcept
    {
        tt_assume(flatVertices != nullptr);
        flatVertices->emplace_back(transform * p1, clippingRectangle, fillColor);
        flatVertices->emplace_back(transform * p2, clippingRectangle, fillColor);
        flatVertices->emplace_back(transform * p3, clippingRectangle, fillColor);
        flatVertices->emplace_back(transform * p4, clippingRectangle, fillColor);
    }

    /** Draw a rectangle of one color.
     * This function will draw the given rectangle.
     * This will use the current:
     *  - transform, to transform each corner of the rectangle.
     *  - clippingRectangle
     *  - fillColor
     */
    void drawFilledQuad(aarect r) const noexcept
    {
        drawFilledQuad(r.corner<0>(), r.corner<1>(), r.corner<2>(), r.corner<3>());
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
    void drawBox(aarect box) const noexcept
    {
        tt_assume(boxVertices != nullptr);

        PipelineBox::DeviceShared::placeVertices(
            *boxVertices, transform * box, fillColor, lineWidth, color, cornerShapes, clippingRectangle);
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
     *  - clippingRectangle
     *  - fillColor
     *  - borderSize
     *  - borderColor
     *  - shadowSize
     *  - cornerShapes
     */
    void drawBoxIncludeBorder(aarect rectangle) const noexcept
    {
        tt_assume(boxVertices != nullptr);

        ttlet shrink_value = lineWidth * 0.5f;

        ttlet new_rectangle = shrink(rectangle, shrink_value);

        ttlet new_corner_shapes =
            vec{std::max(0.0f, cornerShapes.x() - shrink_value),
                std::max(0.0f, cornerShapes.y() - shrink_value),
                std::max(0.0f, cornerShapes.z() - shrink_value),
                std::max(0.0f, cornerShapes.w() - shrink_value)};

        PipelineBox::DeviceShared::placeVertices(
            *boxVertices, transform * new_rectangle, fillColor, lineWidth, color, new_corner_shapes, clippingRectangle);
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
    void drawBoxExcludeBorder(aarect rectangle) const noexcept
    {
        tt_assume(boxVertices != nullptr);

        ttlet shrink_value = lineWidth * 0.5f;

        ttlet new_rectangle = expand(rectangle, shrink_value);

        ttlet new_corner_shapes =
            vec{std::max(0.0f, cornerShapes.x() - shrink_value),
                std::max(0.0f, cornerShapes.y() - shrink_value),
                std::max(0.0f, cornerShapes.z() - shrink_value),
                std::max(0.0f, cornerShapes.w() - shrink_value)};

        PipelineBox::DeviceShared::placeVertices(
            *boxVertices, transform * new_rectangle, fillColor, lineWidth, color, new_corner_shapes, clippingRectangle);
    }

    /** Draw an image
     * This function will draw an image.
     * This will use the current:
     *  - transform, to transform the image.
     *  - clippingRectangle
     */
    void drawImage(PipelineImage::Image &image) const noexcept
    {
        tt_assume(imageVertices != nullptr);

        image.placeVertices(*imageVertices, transform, clippingRectangle);
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
    void drawText(ShapedText const &text, bool useContextColor = false) const noexcept
    {
        tt_assume(sdfVertices != nullptr);

        if (useContextColor) {
            narrow_cast<gui_device_vulkan&>(device()).SDFPipeline->placeVertices(*sdfVertices, text, transform, clippingRectangle, color);
        } else {
            narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->placeVertices(
                *sdfVertices, text, transform, clippingRectangle);
        }
    }

    void drawGlyph(FontGlyphIDs const &glyph, aarect box) const noexcept
    {
        tt_assume(sdfVertices != nullptr);

        narrow_cast<gui_device_vulkan &>(device()).SDFPipeline->placeVertices(
            *sdfVertices, glyph, transform * box, color, clippingRectangle);
    }
};

} // namespace tt
