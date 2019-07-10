// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "attributes.hpp"
#include "TTauriIconParser.hpp"
#include "TTauri/Color.hpp"
#include "TTauri/ResourceView.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace TTauri::Draw {

struct BezierCurve;
struct PathString;
struct Font;
template<typename T> struct PixelMap;


struct Path {
    std::vector<BezierPoint> points;
    std::vector<size_t> contourEndPoints;
    std::vector<std::pair<size_t,wsRGBA>> layerEndContours;

    /*! Bounding box of the path.
     */
    rect2 boundingBox = {};

    /*! This is the position where the left side of the glyph
     * starts. This includes some leading white space so that the glyph
     * will stand a small distance of the edge.
     *
     * For many glyphs the leftSideBearing is the origin.
     */
    glm::vec2 leftSideBearing = {0.0f, 0.0f};

    /*! This is the position where the right side of the glyph
     * ends. This includes some leading white space so that the glyph
     * will stand a small distance of the edge.
     */
    glm::vec2 rightSideBearing = {0.0f, 0.0f};

    /*! Distance from baseline of highest ascender.
     */
    glm::vec2 ascender = {0.0f, 0.0f};

    /*! Distance from baseline of lowest descender.
     */
    glm::vec2 descender = {0.0f, 0.0f};

    /*! Height of capital letter, or height of the letter 'H'.
    */
    glm::vec2 capHeight = {0.0f, 0.0f};

    /*! Height of the small letter 'x'.
    */
    glm::vec2 xHeight = {0.0f, 0.0f};

    /*! The distance to the next character.
     */
    glm::vec2 advance = {0.0f, 0.0f};


    /*! The number of graphemes this glyph represents.
     * This may be larger than one when the glyph is a ligature.
     */
    size_t numberOfGraphemes = 1;

    /*! Get the advanceWidth for the specific grapheme of
     * a potential ligature.
     */
    glm::vec2 advanceForGrapheme(size_t index) const;

    /*! Return the number of closed contours.
    */
    size_t numberOfContours() const;

    /*! Return the number of closed layers.
    */
    size_t numberOfLayers() const;

    /*! Check if all layers have the same color.
     */
    bool allLayersHaveSameColor() const;

    /*! Try to move the layers in a path.
     * Layers are removed if there are layers, and all the layers have
     * the same color.
     */
    void tryRemoveLayers();

    /*! Return an iterator to the start point of a contour.
     */
    std::vector<BezierPoint>::const_iterator beginContour(size_t contourNr) const;

    /* Return and end-iterator beyond the end point of a contour.
     */
    std::vector<BezierPoint>::const_iterator endContour(size_t contourNr) const;

    /* Return the first contour index of a layer.
     */
    size_t beginLayer(size_t layerNr) const;

    /* Return beyond the last contour index of a layer.
     */
    size_t endLayer(size_t layerNr) const;

    std::vector<BezierPoint> getBezierPointsOfContour(size_t contourNr) const;

    std::vector<BezierCurve> getBeziersOfContour(size_t contourNr) const;

    std::vector<BezierCurve> getBeziers() const;

    std::pair<Path,wsRGBA> getLayer(size_t layerNr) const;

    wsRGBA getColorOfLayer(size_t layerNr) const;

    void setColorOfLayer(size_t layerNr, wsRGBA fillColor);

    /*! Return true if there is an open contour.
     */
    bool isContourOpen() const;

    /*! Close current contour.
    * No operation if there is no open contour.
    */
    void closeContour();

    /*! This path has layers.
     */
    bool hasLayers() const;

    /*! Return true if there is an open layer.
    */
    bool isLayerOpen() const;

    /*! Close current contour.
    * No operation if there is no open layer.
    */
    void closeLayer(wsRGBA fillColor);

    /*! Get the currentPosition of the open contour.
     * Returns {0, 0} when there is no contour open.
     */
    glm::vec2 currentPosition() const;

    /*! Start a new contour at position.
     * closes current subpath.
     */
    void moveTo(glm::vec2 position);

    /*! Start a new contour relative to current position.
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

    /*! Draw a circle.
    * \param position position of the center of the circle.
    * \param radius radius of the circle
    */
    void addCircle(glm::vec2 position, float radius);

    /*! Contour with the given bezier curves.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierCurve> const &contour);

    /*! Curve with the given bezier curve.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierPoint>::const_iterator const &begin, std::vector<BezierPoint>::const_iterator const &end);

    /*! Curve with the given bezier curve.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierPoint> const &contour);

    /*! Add path and close layer.
     */
    void addPath(Path const &path, wsRGBA fillColor);

    /*! Stroke a path and close layer.
     */
    void addStroke(Path const &path, wsRGBA strokeColor, float strokeWidth, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f);

    /*! Convert path to stroke-path.
     *
     * This function will create contours that are offset from the original path
     * which creates a stroke. The path will first be subdivided until the curves
     * are mostly flat, then the curves are converted into lines and offset, then
     * the lines are connected to each other.
     *
     * \param strokeWidth width of the stroke.
     * \param lineJoinStyle the style of how outside corners of a stroke are drawn.
     * \param tolerance Tolerance of how flat the curves in the path need to be.
     */
    Path toStroke(float strokeWidth=1.0f, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f) const;
};

Path operator+(Path lhs, Path const &rhs);

Path &operator+=(Path &lhs, Path const &rhs);

Path operator*(glm::mat3x3 const &lhs, Path rhs);

Path &operator*=(Path &lhs, glm::mat3x3 const &rhs);

Path operator*(float const lhs, Path rhs);

Path &operator*=(Path &lhs, float const rhs);

Path operator+(glm::vec2 const &lhs, Path rhs);

Path operator+(Path lhs, glm::vec2 const &rhs);

Path &operator+=(Path &lhs, glm::vec2 const &rhs);


/*! Composit color onto the destination image where the mask is solid.
 *
 * \param dst destination image.
 * \param color color to composit.
 * \param mask mask where the color will be composited on the destination.
 * \param subpixel orientation to improve resolution on LCD displays.
 */
void composit(PixelMap<wsRGBA>& dst, wsRGBA color, Path const &mask, SubpixelOrientation subpixelOrientation);

/*! Composit color onto the destination image where the mask is solid.
*
* \param dst destination image.
* \param mask mask where the color will be composited on the destination.
* \param subpixel orientation to improve resolution on LCD displays.
*/
void composit(PixelMap<wsRGBA>& dst, Path const &mask, SubpixelOrientation subpixelOrientation);



}

namespace TTauri {

template<>
inline Draw::Path parseResource(URL const &location)
{
    let view = ResourceView(location);

    if (location.extension() == "tticon") {
        try {
            let bytes = view.bytes();
            return Draw::parseTTauriIcon(bytes);
        } catch (boost::exception &e) {
            e << errinfo_url(location);
            throw;
        }

    } else {
        BOOST_THROW_EXCEPTION(FileError("Unknown extension")
            << errinfo_url(location)
        );
    }
}

}