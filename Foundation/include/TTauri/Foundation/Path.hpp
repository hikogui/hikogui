// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/BezierPoint.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/TTauriIconParser.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/SDF8.hpp"
#include <vector>

namespace TTauri {

struct BezierCurve;
template<typename T> struct PixelMap;

/*! A path is a vector graphics object.
 * It represents:
 *  - a set of layers each with a different color.
 *  - a layer is a set of contours
 *  - a contour is a set of bezier point describing a closed set of bezier curves.
 */
struct Path {
    /*! A set of all bezier points describing all bezier curves, contours and layers.
     */
    std::vector<BezierPoint> points;

    /*! An index into \see points where each contour ends.
     */
    std::vector<ssize_t> contourEndPoints;

    /*! An color and index into \see contourEndPoints where each layer ends.
     */
    std::vector<std::pair<ssize_t,vec>> layerEndContours;

    /** Clear the path.
     */
    void clear() noexcept {
        points.clear();
        contourEndPoints.clear();
        layerEndContours.clear();
    }

    /*! Return the number of closed contours.
    */
    [[nodiscard]] ssize_t numberOfContours() const noexcept;

    /*! Return the number of closed layers.
    */
    [[nodiscard]] ssize_t numberOfLayers() const noexcept;

    /*! Check if all layers have the same color.
     */
    [[nodiscard]] bool allLayersHaveSameColor() const noexcept;

    /** Calculate bounding box.
     */
    [[nodiscard]] rect boundingBox() const noexcept;

    /*! Try to move the layers in a path.
     * Layers are removed if there are layers, and all the layers have
     * the same color.
     */
    void tryRemoveLayers() noexcept;

    /*! Return an iterator to the start point of a contour.
     */
    [[nodiscard]] std::vector<BezierPoint>::const_iterator beginContour(ssize_t contourNr) const noexcept;

    /* Return and end-iterator beyond the end point of a contour.
     */
    [[nodiscard]] std::vector<BezierPoint>::const_iterator endContour(ssize_t contourNr) const noexcept;

    /* Return the first contour index of a layer.
     */
    [[nodiscard]] ssize_t beginLayer(ssize_t layerNr) const noexcept;

    /* Return beyond the last contour index of a layer.
     */
    [[nodiscard]] ssize_t endLayer(ssize_t layerNr) const noexcept;

    [[nodiscard]] std::vector<BezierPoint> getBezierPointsOfContour(ssize_t contourNr) const noexcept;

    [[nodiscard]] std::vector<BezierCurve> getBeziersOfContour(ssize_t contourNr) const noexcept;

    [[nodiscard]] std::vector<BezierCurve> getBeziers() const noexcept;

    [[nodiscard]] std::pair<Path,vec> getLayer(ssize_t layerNr) const noexcept;

    [[nodiscard]] vec getColorOfLayer(ssize_t layerNr) const noexcept;

    void setColorOfLayer(ssize_t layerNr, vec fillColor) noexcept;

    /*! Return true if there is an open contour.
     */
    [[nodiscard]] bool isContourOpen() const noexcept;

    /*! Close current contour.
    * No operation if there is no open contour.
    */
    void closeContour() noexcept;

    /*! This path has layers.
     */
    [[nodiscard]] bool hasLayers() const noexcept;

    /*! Return true if there is an open layer.
    */
    [[nodiscard]] bool isLayerOpen() const noexcept;

    /*! Close current contour.
    * No operation if there is no open layer.
    */
    void closeLayer(vec fillColor) noexcept;

    /** Optimize layers.
     * Merge contiguous layers with the same color.
     */
    void optimizeLayers() noexcept;

    /*! Get the currentPosition of the open contour.
     * Returns {0, 0} when there is no contour open.
     */
    [[nodiscard]] vec currentPosition() const noexcept;

    /*! Start a new contour at position.
     * closes current subpath.
     */
    void moveTo(vec position) noexcept;

    /*! Start a new contour relative to current position.
     * closes current subpath.
     */
    void moveRelativeTo(vec direction) noexcept;

    void lineTo(vec position) noexcept;

    void lineRelativeTo(vec direction) noexcept;

    void quadraticCurveTo(vec controlPosition, vec position) noexcept;

    /*! Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void quadraticCurveRelativeTo(vec controlDirection, vec direction) noexcept;

    void cubicCurveTo(vec controlPosition1, vec controlPosition2, vec position) noexcept;

    /*! Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void cubicCurveRelativeTo(vec controlDirection1, vec controlDirection2, vec direction) noexcept;

    /*! Draw an circular arc.
     * The arc is drawn from the current position to the position given
     * in this method. A positive arc is drawn counter-clockwise.
     *
     * Using method in:
     *     "Approximation of a cubic bezier curve by circular arcs and vice versa"
     *     -- Aleksas Riškus (chapter 3, formulas 8 and 9, there are a few typos in the formulas)
     *
     * \param radius positive radius means positive arc, negative radius is a negative arc.
     * \param position end position of the arc.
     */
    void arcTo(float radius, vec position) noexcept;

    /*! Draw a rectangle.
     * \param rect the offset and size of the rectangle.
     * \param corner radius of <bottom-left, bottom-right, top-left, top-right>
     *        positive corner are rounded, negative curves are cut.
     */
    void addRectangle(rect r, vec corners={0.0f, 0.0f, 0.0f, 0.0f}) noexcept;

    /*! Draw a circle.
    * \param position position of the center of the circle.
    * \param radius radius of the circle
    */
    void addCircle(vec position, float radius) noexcept;

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
    void addPath(Path const &path, vec fillColor) noexcept;

    /*! Stroke a path and close layer.
     */
    void addStroke(Path const &path, vec strokeColor, float strokeWidth, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f) noexcept;

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
    [[nodiscard]] Path toStroke(float strokeWidth=1.0f, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f) const noexcept;

    /** Center and scale a path inside the extent with padding.
     */
    [[nodiscard]] Path centerScale(vec extent, float padding=0.0) const noexcept;
};

Path operator+(Path lhs, Path const &rhs) noexcept;

Path &operator+=(Path &lhs, Path const &rhs) noexcept;

Path operator*(mat const &lhs, Path rhs) noexcept;

Path operator+(vec const &lhs, Path rhs) noexcept;

Path &operator*=(Path &lhs, mat const &rhs) noexcept;

Path &operator+=(Path &lhs, vec const &rhs) noexcept;


/*! Composit color onto the destination image where the mask is solid.
 *
 * \param dst destination image.
 * \param color color to composit.
 * \param mask mask where the color will be composited on the destination.
 * \param subpixel orientation to improve resolution on LCD displays.
 */
void composit(PixelMap<wsRGBA>& dst, vec color, Path const &mask, SubpixelOrientation subpixelOrientation) noexcept;

/*! Composit color onto the destination image where the mask is solid.
*
* \param dst destination image.
* \param mask mask where the color will be composited on the destination.
* \param subpixel orientation to improve resolution on LCD displays.
*/
void composit(PixelMap<wsRGBA>& dst, Path const &mask, SubpixelOrientation subpixelOrientation) noexcept;

/** Fill a signed distance field image from the given path.
* @param image An signed-distance-field which show distance toward the closest curve
* @param path A path.
*/
void fill(PixelMap<SDF8> &dst, Path const &path) noexcept;

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
