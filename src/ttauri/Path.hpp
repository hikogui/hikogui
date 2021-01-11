// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "BezierCurve.hpp"
#include "ResourceView.hpp"
#include "exception.hpp"
#include "numeric_array.hpp"
#include "aarect.hpp"
#include "mat.hpp"
#include "R16G16B16A16SFloat.hpp"
#include "SDF8.hpp"
#include <vector>

namespace tt {

struct BezierCurve;
template<typename T> class pixel_map;

/** A path is a vector graphics object.
 * It represents:
 *  - a set of layers each with a different color.
 *  - a layer is a set of contours
 *  - a contour is a set of bezier point describing a closed set of bezier curves.
 */
struct Path {
    /** A set of all bezier points describing all bezier curves, contours and layers.
     */
    std::vector<BezierPoint> points;

    /** An index into \see points where each contour ends.
     */
    std::vector<ssize_t> contourEndPoints;

    /** An color and index into \see contourEndPoints where each layer ends.
     */
    std::vector<std::pair<ssize_t,f32x4>> layerEndContours;

    /** Clear the path.
     */
    void clear() noexcept {
        points.clear();
        contourEndPoints.clear();
        layerEndContours.clear();
    }

    /** Return the number of closed contours.
    */
    [[nodiscard]] ssize_t numberOfContours() const noexcept;

    /** Return the number of closed layers.
    */
    [[nodiscard]] ssize_t numberOfLayers() const noexcept;

    /** Check if all layers have the same color.
     */
    [[nodiscard]] bool allLayersHaveSameColor() const noexcept;

    /** Calculate bounding box.
     */
    [[nodiscard]] aarect boundingBox() const noexcept;

    /** Try to move the layers in a path.
     * Layers are removed if there are layers, and all the layers have
     * the same color.
     */
    void tryRemoveLayers() noexcept;

    /** Return an iterator to the start point of a contour.
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

    [[nodiscard]] std::pair<Path,f32x4> getLayer(ssize_t layerNr) const noexcept;

    [[nodiscard]] f32x4 getColorOfLayer(ssize_t layerNr) const noexcept;

    void setColorOfLayer(ssize_t layerNr, f32x4 fillColor) noexcept;

    /** Return true if there is an open contour.
     */
    [[nodiscard]] bool isContourOpen() const noexcept;

    /** Close current contour.
    * No operation if there is no open contour.
    */
    void closeContour() noexcept;

    /** This path has layers.
     */
    [[nodiscard]] bool hasLayers() const noexcept;

    /** Return true if there is an open layer.
    */
    [[nodiscard]] bool isLayerOpen() const noexcept;

    /** Close current contour.
    * No operation if there is no open layer.
    */
    void closeLayer(f32x4 fillColor) noexcept;

    /** Optimize layers.
     * Merge contiguous layers with the same color.
     */
    void optimizeLayers() noexcept;

    /** Get the currentPosition of the open contour.
     * Returns {0, 0} when there is no contour open.
     */
    [[nodiscard]] f32x4 currentPosition() const noexcept;

    /** Start a new contour at position.
     * closes current subpath.
     */
    void moveTo(f32x4 position) noexcept;

    /** Start a new contour relative to current position.
     * closes current subpath.
     */
    void moveRelativeTo(f32x4 direction) noexcept;

    void lineTo(f32x4 position) noexcept;

    void lineRelativeTo(f32x4 direction) noexcept;

    void quadraticCurveTo(f32x4 controlPosition, f32x4 position) noexcept;

    /** Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void quadraticCurveRelativeTo(f32x4 controlDirection, f32x4 direction) noexcept;

    void cubicCurveTo(f32x4 controlPosition1, f32x4 controlPosition2, f32x4 position) noexcept;

    /** Draw curve from the current position to the new direction.
     * @param controlDirection1 The first control point of the curve relative from the start of the curve.
     * @param controlDirection2 The second control point of the curve relative from the start of the curve.
     * @param direction end point of the curve relative from the start of the curve.
     */
    void cubicCurveRelativeTo(f32x4 controlDirection1, f32x4 controlDirection2, f32x4 direction) noexcept;

    /** Draw an circular arc.
     * The arc is drawn from the current position to the position given
     * in this method. A positive arc is drawn counter-clockwise.
     *
     * Using method in:
     *     "Approximation of a cubic bezier curve by circular arcs and vice versa"
     *     -- Aleksas Riskus (chapter 3, formulas 8 and 9, there are a few typos in the formulas)
     *
     * \param radius positive radius means positive arc, negative radius is a negative arc.
     * \param position end position of the arc.
     */
    void arcTo(float radius, f32x4 position) noexcept;

    /** Draw a rectangle.
     * \param rectangle the offset and size of the rectangle.
     * \param corners The radius of the (bottom-left, bottom-right, top-left, top-right)
     *        positive corner are rounded, negative curves are cut.
     */
    void addRectangle(aarect rectangle, f32x4 corners={0.0f, 0.0f, 0.0f, 0.0f}) noexcept;

    /** Draw a circle.
    * \param position position of the center of the circle.
    * \param radius radius of the circle
    */
    void addCircle(f32x4 position, float radius) noexcept;

    /** Contour with the given bezier curves.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierCurve> const &contour) noexcept;

    /** Curve with the given bezier curve.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierPoint>::const_iterator const &begin, std::vector<BezierPoint>::const_iterator const &end) noexcept;

    /** Curve with the given bezier curve.
    * The first anchor will be ignored.
    */
    void addContour(std::vector<BezierPoint> const &contour) noexcept;

    /** Add path and close layer.
     */
    void addPath(Path const &path, f32x4 fillColor) noexcept;

    /** Stroke a path and close layer.
     */
    void addStroke(Path const &path, f32x4 strokeColor, float strokeWidth, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter, float tolerance=0.05f) noexcept;

    /** Convert path to stroke-path.
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
    [[nodiscard]] Path centerScale(f32x4 extent, float padding=0.0) const noexcept;

    Path &operator+=(Path const &rhs) noexcept;

    [[nodiscard]] friend Path operator+(Path lhs, Path const &rhs) noexcept {
        return lhs += rhs;
    }

    template<typename M, std::enable_if_t<is_mat_v<M>, int> = 0>
    Path &operator*=(M const &rhs) noexcept {
        for (auto &&point: points) {
            point *= rhs;
        }
        return *this;
    }

    template<typename M, std::enable_if_t<is_mat_v<M>, int> = 0>
    friend Path operator*(M const &lhs, Path rhs) noexcept {
        return rhs *= lhs;
    }
};





/** Composit color onto the destination image where the mask is solid.
*
* \param dst destination image.
* \param color color to composit.
* \param mask mask where the color will be composited on the destination.
*/
void composit(pixel_map<R16G16B16A16SFloat>& dst, f32x4 color, Path const &mask) noexcept;

/** Composit color onto the destination image where the mask is solid.
*
* \param dst destination image.
* \param mask mask where the color will be composited on the destination.
*/
void composit(pixel_map<R16G16B16A16SFloat>& dst, Path const &mask) noexcept;

/** Fill a signed distance field image from the given path.
* @param dst An signed-distance-field which show distance toward the closest curve
* @param path A path.
*/
void fill(pixel_map<SDF8> &dst, Path const &path) noexcept;

}
