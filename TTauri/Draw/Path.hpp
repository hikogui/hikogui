#pragma once

#include "BezierPoint.hpp"
#include "attributes.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace TTauri {
struct wsRGBApm;
}

namespace TTauri::Draw {

struct Bezier;
struct Glyph;
struct Font;
template<typename T> struct PixelMap;


struct Path {
    std::vector<BezierPoint> points;
    std::vector<size_t> endPoints;

    /*! Return the number of closed sub-paths.
     */
    size_t numberOfContours() const;

    std::vector<BezierPoint> getBezierPointsOfContour(size_t subpathNr) const;

    std::vector<Bezier> getBeziersOfContour(size_t subpathNr) const;

    std::vector<Bezier> getBeziers() const;

    /*! Return true if there is an open sub-path.
     */
    bool hasCurrentPosition() const;

    /*! Get the currentPosition of the open sub-path.
     * Returns {0, 0} when there is no sub-path open.
     */
    glm::vec2 currentPosition() const;

    /*! Close current sub-path.
     * No operation if there is no open sub-path.
     */
    void close();

    /*! Start a new sub-path at position.
     * closes current subpath.
     */
    void moveTo(glm::vec2 position);

    /*! Start a new sub-path relative to current position.
     * closes current subpath.
     */
    void moveRelativeTo(glm::vec2 direction);

    void lineTo(glm::vec2 position);

    void lineRelativeTo(glm::vec2 direction);

    void quadraticCurveTo(glm::vec2 controlPosition, glm::vec2 position);

    /*! Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void quadraticCurveRelativeTo(glm::vec2 controlDirection, glm::vec2 direction);

    void cubicCurveTo(glm::vec2 controlPosition1, glm::vec2 controlPosition2, glm::vec2 position);

    /*! Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void cubicCurveRelativeTo(glm::vec2 controlDirection1, glm::vec2 controlDirection2, glm::vec2 direction);

    /*! Draw an circular arc.
     * The arc is drawn from the current position to the position given
     * in this method. A positive arc is drawn counter-clockwise.
     *
     * Using method in:
     *     "Approximation of a cubic bezier curve by circular arcs and vice versa"
     *     -- Aleksas Riškus (chapter 3, formulas 8 and 9, there are a few typos in the formulas)
     *
     * \param radius postive radius means positive arc, negative radius is a negative arc.
     * \param position end position of the arc.
     */
    void arcTo(float radius, glm::vec2 position);

    /*! Draw a rectangle.
     * \param rect the offset and size of the rectangle.
     * \param corner radius of <bottom-left, bottom-right, top-right, top-left>
     *        positive corner are rounded, negative curves are cut.
     */
    void addRectangle(rect2 rect, glm::vec4 corners={0.0f, 0.0f, 0.0f, 0.0f});

    /*! Add glyph to path.
     * \param glyph Glyph to draw.
     * \param position The position to draw the origin of the glyph.
     * \param scale how much to scale the glyph by, the original glyph is 1Em high.
     * \param rotation Rotation in radials clock wise.
     */
    void addGlyph(Glyph const &glyph, glm::vec2 position, float scale, float rotation = 0.0f);

    void addText(std::string const &text, Font const &font, glm::vec2 position, float scale, float rotation = 0.0f, HorizontalAlignment alignment=HorizontalAlignment::Left);

    /*! Curve with the given bezier curve.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<Bezier> const &contour);

    /*! Add a path to stroke into this path.
     *
     * This function will create contours that are offset from the original path
     * which creates a stroke. The path will first be subdivided until the curves
     * are mostly flat, then the curves are converted into lines and offset, then
     * the lines are connected to each other.
     *
     * \param path path to stroke.
     * \param strokeWidth width of the stroke.
     * \param lineJoinStyle the style of how outside corners of a stroke are drawn.
     * \param tolerance Tolerance of how flat the curves in the path need to be.
     */
    void addPathToStroke(Path const &path, float strokeWidth, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f);
};

/*! Composit color onto the destination image where the mask is solid.
 *
 * \param dst destination image.
 * \param color color to composit.
 * \param mask mask where the color will be composited on the destination.
 * \param subpixel orientation to improve resolution on LCD displays.
 */
void fill(PixelMap<wsRGBApm>& dst, wsRGBApm color, Path const &mask, SubpixelOrientation subpixelOrientation);

/*! Composit color onto the destination image on the edges of the mask.
 *
 * This will internally create a new path; offset from the original mask, which in turn will
 * be filled.
 *
 * \param dst destination image.
 * \param color color to composit.
 * \param mask mask where the color will be composited on the destination.
 * \param strokeWidth the width of the edge of the mask.
 * \param lineJoinStyle the style of how outside corners of a stroke are drawn.
 * \param subpixel orientation to improve resolution on LCD displays.
 */
void stroke(
    PixelMap<wsRGBApm>& dst,
    wsRGBApm color,
    Path const &mask,
    float strokeWidth=1.0f,
    LineJoinStyle lineJoinStyle=LineJoinStyle::Miter,
    SubpixelOrientation subpixelOrientation=SubpixelOrientation::Unknown
);

/*! Composit color onto the destination image on the edges of the mask.
*
* This will internally create a new path; offset from the original mask, which in turn will
* be filled.
*
* \param dst destination image.
* \param color color to composit.
* \param mask mask where the color will be composited on the destination.
* \param strokeWidth the width of the edge of the mask.
* \param lineJoinStyle the style of how outside corners of a stroke are drawn.
* \param subpixel orientation to improve resolution on LCD displays.
*/
void stroke(
    PixelMap<wsRGBApm>& dst,
    wsRGBApm color,
    Path const &mask,
    float strokeWidth=1.0f,
    SubpixelOrientation subpixelOrientation=SubpixelOrientation::Unknown
);

}
