// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/BezierPoint.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/TTauriIconParser.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/GlyphMetrics.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace TTauri {

struct BezierCurve;
struct PathString;
class Font;
template<typename T> struct PixelMap;


struct Path {
    std::vector<BezierPoint> points;
    std::vector<ssize_t> contourEndPoints;
    std::vector<std::pair<ssize_t,wsRGBA>> layerEndContours;

    GlyphMetrics metrics;

    /*! Return the number of closed contours.
    */
    ssize_t numberOfContours() const noexcept;

    /*! Return the number of closed layers.
    */
    ssize_t numberOfLayers() const noexcept;

    /*! Check if all layers have the same color.
     */
    bool allLayersHaveSameColor() const noexcept;

    /*! Try to move the layers in a path.
     * Layers are removed if there are layers, and all the layers have
     * the same color.
     */
    void tryRemoveLayers() noexcept;

    /*! Return an iterator to the start point of a contour.
     */
    std::vector<BezierPoint>::const_iterator beginContour(ssize_t contourNr) const noexcept;

    /* Return and end-iterator beyond the end point of a contour.
     */
    std::vector<BezierPoint>::const_iterator endContour(ssize_t contourNr) const noexcept;

    /* Return the first contour index of a layer.
     */
    ssize_t beginLayer(ssize_t layerNr) const noexcept;

    /* Return beyond the last contour index of a layer.
     */
    ssize_t endLayer(ssize_t layerNr) const noexcept;

    std::vector<BezierPoint> getBezierPointsOfContour(ssize_t contourNr) const noexcept;

    std::vector<BezierCurve> getBeziersOfContour(ssize_t contourNr) const noexcept;

    std::vector<BezierCurve> getBeziers() const noexcept;

    std::pair<Path,wsRGBA> getLayer(ssize_t layerNr) const noexcept;

    wsRGBA getColorOfLayer(ssize_t layerNr) const noexcept;

    void setColorOfLayer(ssize_t layerNr, wsRGBA fillColor) noexcept;

    /*! Return true if there is an open contour.
     */
    bool isContourOpen() const noexcept;

    /*! Close current contour.
    * No operation if there is no open contour.
    */
    void closeContour() noexcept;

    /*! This path has layers.
     */
    bool hasLayers() const noexcept;

    /*! Return true if there is an open layer.
    */
    bool isLayerOpen() const noexcept;

    /*! Close current contour.
    * No operation if there is no open layer.
    */
    void closeLayer(wsRGBA fillColor) noexcept;

    /*! Get the currentPosition of the open contour.
     * Returns {0, 0} when there is no contour open.
     */
    glm::vec2 currentPosition() const noexcept;

    /*! Start a new contour at position.
     * closes current subpath.
     */
    void moveTo(glm::vec2 position) noexcept;

    /*! Start a new contour relative to current position.
     * closes current subpath.
     */
    void moveRelativeTo(glm::vec2 direction) noexcept;

    void lineTo(glm::vec2 position) noexcept;

    void lineRelativeTo(glm::vec2 direction) noexcept;

    void quadraticCurveTo(glm::vec2 controlPosition, glm::vec2 position) noexcept;

    /*! Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void quadraticCurveRelativeTo(glm::vec2 controlDirection, glm::vec2 direction) noexcept;

    void cubicCurveTo(glm::vec2 controlPosition1, glm::vec2 controlPosition2, glm::vec2 position) noexcept;

    /*! Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void cubicCurveRelativeTo(glm::vec2 controlDirection1, glm::vec2 controlDirection2, glm::vec2 direction) noexcept;

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
    void arcTo(float radius, glm::vec2 position) noexcept;

    /*! Draw a rectangle.
     * \param rect the offset and size of the rectangle.
     * \param corner radius of <bottom-left, bottom-right, top-right, top-left>
     *        positive corner are rounded, negative curves are cut.
     */
    void addRectangle(rect2 rect, glm::vec4 corners={0.0f, 0.0f, 0.0f, 0.0f}) noexcept;

    /*! Draw a circle.
    * \param position position of the center of the circle.
    * \param radius radius of the circle
    */
    void addCircle(glm::vec2 position, float radius) noexcept;

    /*! Contour with the given bezier curves.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierCurve> const &contour) noexcept;

    /*! Curve with the given bezier curve.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierPoint>::const_iterator const &begin, std::vector<BezierPoint>::const_iterator const &end) noexcept;

    /*! Curve with the given bezier curve.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierPoint> const &contour) noexcept;

    /*! Add path and close layer.
     */
    void addPath(Path const &path, wsRGBA fillColor) noexcept;

    /*! Stroke a path and close layer.
     */
    void addStroke(Path const &path, wsRGBA strokeColor, float strokeWidth, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f) noexcept;

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
    Path toStroke(float strokeWidth=1.0f, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f) const noexcept;
};

Path operator+(Path lhs, Path const &rhs) noexcept;

Path &operator+=(Path &lhs, Path const &rhs) noexcept;

Path operator*(glm::mat3x3 const &lhs, Path rhs) noexcept;

Path &operator*=(Path &lhs, glm::mat3x3 const &rhs) noexcept;

Path operator*(float const lhs, Path rhs) noexcept;

Path &operator*=(Path &lhs, float const rhs) noexcept;

Path operator+(glm::vec2 const &lhs, Path rhs) noexcept;

Path operator+(Path lhs, glm::vec2 const &rhs) noexcept;

Path &operator+=(Path &lhs, glm::vec2 const &rhs) noexcept;


/*! Composit color onto the destination image where the mask is solid.
 *
 * \param dst destination image.
 * \param color color to composit.
 * \param mask mask where the color will be composited on the destination.
 * \param subpixel orientation to improve resolution on LCD displays.
 */
void composit(PixelMap<wsRGBA>& dst, wsRGBA color, Path const &mask, SubpixelOrientation subpixelOrientation) noexcept;

/*! Composit color onto the destination image where the mask is solid.
*
* \param dst destination image.
* \param mask mask where the color will be composited on the destination.
* \param subpixel orientation to improve resolution on LCD displays.
*/
void composit(PixelMap<wsRGBA>& dst, Path const &mask, SubpixelOrientation subpixelOrientation) noexcept;



}

namespace TTauri {

template<>
inline std::unique_ptr<Path> parseResource(URL const &location)
{
    if (location.extension() == "tticon") {
        let &view = ResourceView::loadView(location);

        try {
            let bytes = view->bytes();
            return std::make_unique<Path>(parseTTauriIcon(bytes));
        } catch (error &e) {
            e.set<"url"_tag>(location);
            throw;
        }

    } else {
        TTAURI_THROW(url_error("Unknown extension")
            .set<"url"_tag>(location)
        );
    }
}

}
