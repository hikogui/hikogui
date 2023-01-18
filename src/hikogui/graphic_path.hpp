// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "bezier_point.hpp"
#include "bezier_curve.hpp"
#include "utility/module.hpp"
#include "geometry/module.hpp"
#include "image/module.hpp"
#include <vector>

namespace hi::inline v1 {

struct bezier_curve;
template<typename T>
class pixmap_span;

/** A path is a vector graphics object.
 * It represents:
 *  - a set of layers each with a different color.
 *  - a layer is a set of contours
 *  - a contour is a set of bezier point describing a closed set of bezier curves.
 */
struct graphic_path {
    /** A set of all bezier points describing all bezier curves, contours and layers.
     */
    std::vector<bezier_point> points;

    /** An index into \see points where each contour ends.
     */
    std::vector<ssize_t> contourEndPoints;

    /** An color and index into \see contourEndPoints where each layer ends.
     */
    std::vector<std::pair<ssize_t, color>> layerEndContours;

    /** Clear the path.
     */
    void clear() noexcept
    {
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
    [[nodiscard]] aarectangle boundingBox() const noexcept;

    /** Try to move the layers in a path.
     * Layers are removed if there are layers, and all the layers have
     * the same color.
     */
    void tryRemoveLayers() noexcept;

    /** Return an iterator to the start point of a contour.
     */
    [[nodiscard]] std::vector<bezier_point>::const_iterator beginContour(ssize_t contourNr) const noexcept;

    /* Return and end-iterator beyond the end point of a contour.
     */
    [[nodiscard]] std::vector<bezier_point>::const_iterator endContour(ssize_t contourNr) const noexcept;

    /* Return the first contour index of a layer.
     */
    [[nodiscard]] ssize_t beginLayer(ssize_t layerNr) const noexcept;

    /* Return beyond the last contour index of a layer.
     */
    [[nodiscard]] ssize_t endLayer(ssize_t layerNr) const noexcept;

    [[nodiscard]] std::vector<bezier_point> getbezier_pointsOfContour(ssize_t contourNr) const noexcept;

    [[nodiscard]] std::vector<bezier_curve> getBeziersOfContour(ssize_t contourNr) const noexcept;

    [[nodiscard]] std::vector<bezier_curve> getBeziers() const noexcept;

    [[nodiscard]] std::pair<graphic_path, color> getLayer(ssize_t layerNr) const noexcept;

    [[nodiscard]] color getColorOfLayer(ssize_t layerNr) const noexcept;

    void setColorOfLayer(ssize_t layerNr, color fill_color) noexcept;

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
    void closeLayer(color fill_color) noexcept;

    /** Optimize layers.
     * Merge contiguous layers with the same color.
     */
    void optimizeLayers() noexcept;

    /** Get the currentPosition of the open contour.
     * Returns {0, 0} when there is no contour open.
     */
    [[nodiscard]] point2 currentPosition() const noexcept;

    /** Start a new contour at position.
     * closes current subpath.
     */
    void moveTo(point2 position) noexcept;

    /** Start a new contour relative to current position.
     * closes current subpath.
     */
    void moveRelativeTo(vector2 direction) noexcept;

    void lineTo(point2 position) noexcept;

    void lineRelativeTo(vector2 direction) noexcept;

    void quadraticCurveTo(point2 controlPosition, point2 position) noexcept;

    /** Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void quadraticCurveRelativeTo(vector2 controlDirection, vector2 direction) noexcept;

    void cubicCurveTo(point2 controlPosition1, point2 controlPosition2, point2 position) noexcept;

    /** Draw curve from the current position to the new direction.
     * @param controlDirection1 The first control point of the curve relative from the start of the curve.
     * @param controlDirection2 The second control point of the curve relative from the start of the curve.
     * @param direction end point of the curve relative from the start of the curve.
     */
    void cubicCurveRelativeTo(vector2 controlDirection1, vector2 controlDirection2, vector2 direction) noexcept;

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
    void arcTo(float radius, point2 position) noexcept;

    /** Draw a rectangle.
     * \param rectangle the offset and size of the rectangle.
     * \param corners The radius of the (bottom-left, bottom-right, top-left, top-right)
     *        positive corner are rounded, negative curves are cut.
     */
    void addRectangle(aarectangle rectangle, corner_radii corners = corner_radii{0.0f, 0.0f, 0.0f, 0.0f}) noexcept;

    /** Draw a circle.
     * \param position position of the center of the circle.
     * \param radius radius of the circle
     */
    void addCircle(point2 position, float radius) noexcept;

    /** Contour with the given bezier curves.
     * The first anchor will be ignored.
     */
    void addContour(std::vector<bezier_curve> const &contour) noexcept;

    /** Curve with the given bezier curve.
     * The first anchor will be ignored.
     */
    void addContour(
        std::vector<bezier_point>::const_iterator const &begin,
        std::vector<bezier_point>::const_iterator const &end) noexcept;

    /** Curve with the given bezier curve.
     * The first anchor will be ignored.
     */
    void addContour(std::vector<bezier_point> const &contour) noexcept;

    /** Add path and close layer.
     */
    void addPath(graphic_path const &path, color fill_color) noexcept;

    /** Stroke a path and close layer.
     */
    void addStroke(
        graphic_path const &path,
        color strokeColor,
        float strokeWidth,
        hi::line_join_style line_join_style = line_join_style::miter,
        float tolerance = 0.05f) noexcept;

    /** Convert path to stroke-path.
     *
     * This function will create contours that are offset from the original path
     * which creates a stroke. The path will first be subdivided until the curves
     * are mostly flat, then the curves are converted into lines and offset, then
     * the lines are connected to each other.
     *
     * \param strokeWidth width of the stroke.
     * \param line_join_style the style of how outside corners of a stroke are drawn.
     * \param tolerance Tolerance of how flat the curves in the path need to be.
     */
    [[nodiscard]] graphic_path toStroke(
        float strokeWidth = 1.0f,
        line_join_style line_join_style = line_join_style::miter,
        float tolerance = 0.05f) const noexcept;

    /** Center and scale a path inside the extent with padding.
     */
    [[nodiscard]] graphic_path centerScale(extent2 extent, float padding = 0.0) const noexcept;

    graphic_path &operator+=(graphic_path const &rhs) noexcept;

    [[nodiscard]] friend graphic_path operator+(graphic_path lhs, graphic_path const &rhs) noexcept
    {
        return lhs += rhs;
    }

    friend graphic_path operator*(geo::transformer auto const &lhs, graphic_path const &rhs) noexcept
    {
        auto rhs_ = rhs;
        for (auto &&point : rhs_.points) {
            point = lhs * point;
        }
        return rhs_;
    }
};

/** Composit color onto the destination image where the mask is solid.
 *
 * \param dst destination image.
 * \param color color to composit.
 * \param mask mask where the color will be composited on the destination.
 */
void composit(pixmap_span<sfloat_rgba16> dst, hi::color color, graphic_path const &mask) noexcept;

/** Composit color onto the destination image where the mask is solid.
 *
 * \param dst destination image.
 * \param mask mask where the color will be composited on the destination.
 */
void composit(pixmap_span<sfloat_rgba16> dst, graphic_path const &mask) noexcept;

/** Fill a signed distance field image from the given path.
 * @param dst An signed-distance-field which show distance toward the closest curve
 * @param path A path.
 */
void fill(pixmap_span<sdf_r8> dst, graphic_path const &path) noexcept;

} // namespace hi::inline v1
