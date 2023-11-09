// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vector>
#include <utility>
#include <cmath>

export module hikogui_graphic_path;
import hikogui_color;
import hikogui_geometry;
export import hikogui_graphic_path_bezier;
export import hikogui_graphic_path_bezier_curve;
export import hikogui_graphic_path_bezier_point;
import hikogui_image;
import hikogui_utility;

export namespace hi { inline namespace v1 {

/** A path is a vector graphics object.
 * It represents:
 *  - a set of layers each with a different color.
 *  - a layer is a set of contours
 *  - a contour is a set of bezier point describing a closed set of bezier curves.
 */
export struct graphic_path {
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
    [[nodiscard]] ssize_t numberOfContours() const noexcept
    {
        return ssize(contourEndPoints);
    }

    /** Return the number of closed layers.
     */
    [[nodiscard]] ssize_t numberOfLayers() const noexcept
    {
        return ssize(layerEndContours);
    }

    /** Check if all layers have the same color.
     */
    [[nodiscard]] bool allLayersHaveSameColor() const noexcept
    {
        if (!hasLayers()) {
            return true;
        }

        hilet& firstColor = layerEndContours.front().second;

        for (hilet & [ endContour, color ] : layerEndContours) {
            if (color != firstColor) {
                return false;
            }
        }
        return true;
    }

    /** Calculate bounding box.
     */
    [[nodiscard]] aarectangle boundingBox() const noexcept
    {
        if (ssize(points) == 0) {
            return aarectangle{0.0, 0.0, 0.0, 0.0};
        }

        auto r = aarectangle{points.front().p, points.front().p};

        for (hilet& point : points) {
            r |= point.p;
        }

        return r;
    }

    /** Try to move the layers in a path.
     * Layers are removed if there are layers, and all the layers have
     * the same color.
     */
    void tryRemoveLayers() noexcept
    {
        if (!hasLayers()) {
            return;
        }

        if (!allLayersHaveSameColor()) {
            return;
        }

        layerEndContours.clear();
    }

    /** Return an iterator to the start point of a contour.
     */
    [[nodiscard]] std::vector<bezier_point>::const_iterator beginContour(ssize_t contourNr) const noexcept
    {
        return points.begin() + (contourNr == 0 ? 0 : contourEndPoints.at(contourNr - 1) + 1);
    }

    /* Return and end-iterator beyond the end point of a contour.
     */
    [[nodiscard]] std::vector<bezier_point>::const_iterator endContour(ssize_t contourNr) const noexcept
    {
        return points.begin() + contourEndPoints.at(contourNr) + 1;
    }

    /* Return the first contour index of a layer.
     */
    [[nodiscard]] ssize_t beginLayer(ssize_t layerNr) const noexcept
    {
        return layerNr == 0 ? 0 : layerEndContours.at(layerNr - 1).first + 1;
    }

    /* Return beyond the last contour index of a layer.
     */
    [[nodiscard]] ssize_t endLayer(ssize_t layerNr) const noexcept
    {
        return layerEndContours.at(layerNr).first + 1;
    }

    [[nodiscard]] std::vector<bezier_point> getbezier_pointsOfContour(ssize_t contourNr) const noexcept
    {
        hilet begin = points.begin() + (contourNr == 0 ? 0 : contourEndPoints.at(contourNr - 1) + 1);
        hilet end = points.begin() + contourEndPoints.at(contourNr) + 1;
        return std::vector(begin, end);
    }

    [[nodiscard]] std::vector<bezier_curve> getBeziersOfContour(ssize_t contourNr) const noexcept
    {
        auto first = beginContour(contourNr);
        auto last = endContour(contourNr);
        auto num_points = std::distance(first, last);
        if (num_points < 3) {
            // Contours with less than three points do not have volume and are invisible.
            // Contours with one point are used for anchors when compositing compound glyphs.
            return {};
        }

        return makeContourFromPoints(beginContour(contourNr), endContour(contourNr));
    }

    [[nodiscard]] std::vector<bezier_curve> getBeziers() const noexcept
    {
        hi_assert(!hasLayers());

        std::vector<bezier_curve> r;

        for (auto contourNr = 0; contourNr < numberOfContours(); contourNr++) {
            hilet beziers = getBeziersOfContour(contourNr);
            r.insert(r.end(), beziers.begin(), beziers.end());
        }
        return r;
    }

    [[nodiscard]] std::pair<graphic_path, color> getLayer(ssize_t layerNr) const noexcept
    {
        hi_assert(hasLayers());

        auto path = graphic_path{};

        hilet begin = beginLayer(layerNr);
        hilet end = endLayer(layerNr);
        for (ssize_t contourNr = begin; contourNr != end; contourNr++) {
            path.addContour(beginContour(contourNr), endContour(contourNr));
        }

        return {path, getColorOfLayer(layerNr)};
    }

    [[nodiscard]] color getColorOfLayer(ssize_t layerNr) const noexcept
    {
        return layerEndContours.at(layerNr).second;
    }

    void setColorOfLayer(ssize_t layerNr, color fill_color) noexcept
    {
        layerEndContours.at(layerNr).second = fill_color;
    }

    /** Return true if there is an open contour.
     */
    [[nodiscard]] bool isContourOpen() const noexcept
    {
        if (points.size() == 0) {
            return false;
        } else if (contourEndPoints.size() == 0) {
            return true;
        } else {
            return contourEndPoints.back() != (ssize(points) - 1);
        }
    }

    /** Close current contour.
     * No operation if there is no open contour.
     */
    void closeContour() noexcept
    {
        if (isContourOpen()) {
            contourEndPoints.push_back(ssize(points) - 1);
        }
    }

    /** This path has layers.
     */
    [[nodiscard]] bool hasLayers() const noexcept
    {
        return numberOfLayers() > 0;
    }

    /** Return true if there is an open layer.
     */
    [[nodiscard]] bool isLayerOpen() const noexcept
    {
        if (points.size() == 0) {
            return false;
        } else if (isContourOpen()) {
            return true;
        } else if (layerEndContours.size() == 0) {
            return true;
        } else {
            return layerEndContours.back().first != (ssize(contourEndPoints) - 1);
        }
    }

    /** Close current contour.
     * No operation if there is no open layer.
     */
    void closeLayer(color fill_color) noexcept
    {
        closeContour();
        if (isLayerOpen()) {
            layerEndContours.emplace_back(ssize(contourEndPoints) - 1, fill_color);
        }
    }

    /** Optimize layers.
     * Merge contiguous layers with the same color.
     */
    void optimizeLayers() noexcept
    {
        if (ssize(layerEndContours) == 0) {
            return;
        }

        decltype(layerEndContours) tmp;
        tmp.reserve(ssize(layerEndContours));

        auto prev_i = layerEndContours.begin();
        for (auto i = prev_i + 1; i != layerEndContours.end(); ++i) {
            // Add the last layer of a contiguous color.
            if (prev_i->second != i->second) {
                tmp.push_back(*prev_i);
            }

            prev_i = i;
        }
        tmp.push_back(*prev_i);

        std::swap(layerEndContours, tmp);
    }

    /** Get the currentPosition of the open contour.
     * Returns {0, 0} when there is no contour open.
     */
    [[nodiscard]] point2 currentPosition() const noexcept
    {
        if (isContourOpen()) {
            return points.back().p;
        } else {
            return point2{};
        }
    }

    /** Start a new contour at position.
     * closes current subpath.
     */
    void moveTo(point2 position) noexcept
    {
        closeContour();
        points.emplace_back(position, bezier_point::Type::Anchor);
    }

    /** Start a new contour relative to current position.
     * closes current subpath.
     */
    void moveRelativeTo(vector2 direction) noexcept
    {
        hi_assert(isContourOpen());

        hilet lastPosition = currentPosition();
        closeContour();
        points.emplace_back(lastPosition + direction, bezier_point::Type::Anchor);
    }

    void lineTo(point2 position) noexcept
    {
        hi_assert(isContourOpen());

        points.emplace_back(position, bezier_point::Type::Anchor);
    }

    void lineRelativeTo(vector2 direction) noexcept
    {
        hi_assert(isContourOpen());

        points.emplace_back(currentPosition() + direction, bezier_point::Type::Anchor);
    }

    void quadraticCurveTo(point2 controlPosition, point2 position) noexcept
    {
        hi_assert(isContourOpen());

        points.emplace_back(controlPosition, bezier_point::Type::QuadraticControl);
        points.emplace_back(position, bezier_point::Type::Anchor);
    }

    /** Draw curve from the current position to the new direction.
     * \param controlDirection control point of the curve relative from the start of the curve.
     * \param direction end point of the curve relative from the start of the curve.
     */
    void quadraticCurveRelativeTo(vector2 controlDirection, vector2 direction) noexcept
    {
        hi_assert(isContourOpen());

        hilet p = currentPosition();
        points.emplace_back(p + controlDirection, bezier_point::Type::QuadraticControl);
        points.emplace_back(p + direction, bezier_point::Type::Anchor);
    }

    void cubicCurveTo(point2 controlPosition1, point2 controlPosition2, point2 position) noexcept
    {
        hi_assert(isContourOpen());

        points.emplace_back(controlPosition1, bezier_point::Type::CubicControl1);
        points.emplace_back(controlPosition2, bezier_point::Type::CubicControl2);
        points.emplace_back(position, bezier_point::Type::Anchor);
    }

    /** Draw curve from the current position to the new direction.
     * @param controlDirection1 The first control point of the curve relative from the start of the curve.
     * @param controlDirection2 The second control point of the curve relative from the start of the curve.
     * @param direction end point of the curve relative from the start of the curve.
     */
    void cubicCurveRelativeTo(vector2 controlDirection1, vector2 controlDirection2, vector2 direction) noexcept
    {
        hi_assert(isContourOpen());

        hilet p = currentPosition();
        points.emplace_back(p + controlDirection1, bezier_point::Type::CubicControl1);
        points.emplace_back(p + controlDirection2, bezier_point::Type::CubicControl2);
        points.emplace_back(p + direction, bezier_point::Type::Anchor);
    }

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
    void arcTo(float radius, point2 position) noexcept
    {
        hi_assert(isContourOpen());

        hilet r = std::abs(radius);
        hilet P1 = currentPosition();
        hilet P2 = position;
        hilet Pm = midpoint(P1, P2);

        hilet Vm2 = P2 - Pm;

        // Calculate the half angle between vectors P0 - C and P2 - C.
        hilet alpha = std::asin(hypot(Vm2) / r);

        // Calculate the center point C. As the length of the normal of Vm2 at Pm.
        hilet C = Pm + normal(Vm2) * std::cos(alpha) * radius;

        // Calculate vectors from center to end points.
        hilet VC1 = P1 - C;
        hilet VC2 = P2 - C;

        hilet q1 = squared_hypot(VC1);
        hilet q2 = q1 + dot(VC1, VC2);
        hilet k2 = (4.0f / 3.0f) * (std::sqrt(2.0f * q1 * q2) - q2) / cross(VC1, VC2);

        // Calculate the control points.
        hilet C1 = point2{(C.x() + VC1.x()) - k2 * VC1.y(), (C.y() + VC1.y()) + k2 * VC1.x()};
        hilet C2 = point2{(C.x() + VC2.x()) + k2 * VC2.y(), (C.y() + VC2.y()) - k2 * VC2.x()};

        cubicCurveTo(C1, C2, P2);
    }

    /** Draw a rectangle.
     * \param rectangle the offset and size of the rectangle.
     * \param corners The radius of the (bottom-left, bottom-right, top-left, top-right)
     *        positive corner are rounded, negative curves are cut.
     */
    void addRectangle(aarectangle rectangle, corner_radii corners = corner_radii{0.0f, 0.0f, 0.0f, 0.0f}) noexcept
    {
        hi_assert(!isContourOpen());

        hilet bl_radius = std::abs(corners.left_bottom());
        hilet br_radius = std::abs(corners.right_bottom());
        hilet tl_radius = std::abs(corners.left_top());
        hilet tr_radius = std::abs(corners.right_top());

        hilet blc = get<0>(rectangle);
        hilet brc = get<1>(rectangle);
        hilet tlc = get<2>(rectangle);
        hilet trc = get<3>(rectangle);

        hilet blc1 = blc + vector2{0.0f, bl_radius};
        hilet blc2 = blc + vector2{bl_radius, 0.0f};
        hilet brc1 = brc + vector2{-br_radius, 0.0f};
        hilet brc2 = brc + vector2{0.0f, br_radius};
        hilet tlc1 = tlc + vector2{tl_radius, 0.0f};
        hilet tlc2 = tlc + vector2{0.0f, -tl_radius};
        hilet trc1 = trc + vector2{0.0f, -tr_radius};
        hilet trc2 = trc + vector2{-tr_radius, 0.0f};

        moveTo(blc1);
        if (corners.left_bottom() > 0.0) {
            arcTo(bl_radius, blc2);
        } else if (corners.left_bottom() < 0.0) {
            lineTo(blc2);
        }

        lineTo(brc1);
        if (corners.right_bottom() > 0.0) {
            arcTo(br_radius, brc2);
        } else if (corners.right_bottom() < 0.0) {
            lineTo(blc2);
        }

        lineTo(tlc1);
        if (corners.left_top() > 0.0) {
            arcTo(tl_radius, tlc2);
        } else if (corners.left_top() < 0.0) {
            lineTo(tlc2);
        }

        lineTo(trc1);
        if (corners.right_top() > 0.0) {
            arcTo(tr_radius, trc2);
        } else if (corners.right_top() < 0.0) {
            lineTo(trc2);
        }

        closeContour();
    }

    /** Draw a circle.
     * \param position position of the center of the circle.
     * \param radius radius of the circle
     */
    void addCircle(point2 position, float radius) noexcept
    {
        hi_assert(!isContourOpen());

        moveTo(point2{position.x(), position.y() - radius});
        arcTo(radius, point2{position.x() + radius, position.y()});
        arcTo(radius, point2{position.x(), position.y() + radius});
        arcTo(radius, point2{position.x() - radius, position.y()});
        arcTo(radius, point2{position.x(), position.y() - radius});
        closeContour();
    }

    /** Contour with the given bezier curves.
     * The first anchor will be ignored.
     */
    void addContour(std::vector<bezier_curve> const& contour) noexcept
    {
        hi_assert(!isContourOpen());

        for (hilet& curve : contour) {
            // Don't emit the first point, the last point of the contour will wrap around.
            switch (curve.type) {
            case bezier_curve::Type::Linear:
                points.emplace_back(curve.P2, bezier_point::Type::Anchor);
                break;
            case bezier_curve::Type::Quadratic:
                points.emplace_back(curve.C1, bezier_point::Type::QuadraticControl);
                points.emplace_back(curve.P2, bezier_point::Type::Anchor);
                break;
            case bezier_curve::Type::Cubic:
                points.emplace_back(curve.C1, bezier_point::Type::CubicControl1);
                points.emplace_back(curve.C2, bezier_point::Type::CubicControl2);
                points.emplace_back(curve.P2, bezier_point::Type::Anchor);
                break;
            default:
                hi_no_default();
            }
        }

        closeContour();
    }

    /** Curve with the given bezier curve.
     * The first anchor will be ignored.
     */
    void addContour(
        std::vector<bezier_point>::const_iterator const& begin,
        std::vector<bezier_point>::const_iterator const& end) noexcept
    {
        hi_assert(!isContourOpen());
        points.insert(points.end(), begin, end);
        closeContour();
    }

    /** Curve with the given bezier curve.
     * The first anchor will be ignored.
     */
    void addContour(std::vector<bezier_point> const& contour) noexcept
    {
        addContour(contour.begin(), contour.end());
    }

    /** Add path and close layer.
     */
    void addPath(graphic_path const& path, color fill_color) noexcept
    {
        *this += path;
        closeLayer(fill_color);
    }

    /** Stroke a path and close layer.
     */
    void addStroke(
        graphic_path const& path,
        color strokeColor,
        float strokeWidth,
        hi::line_join_style line_join_style = line_join_style::miter,
        float tolerance = 0.05f) noexcept
    {
        *this += path.toStroke(strokeWidth, line_join_style, tolerance);
        closeLayer(strokeColor);
    }

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
        float tolerance = 0.05f) const noexcept
    {
        hi_assert(!hasLayers());
        hi_assert(!isContourOpen());

        auto r = graphic_path{};

        float starboardOffset = strokeWidth / 2;
        float portOffset = -starboardOffset;

        for (int i = 0; i < numberOfContours(); i++) {
            hilet baseContour = getBeziersOfContour(i);

            hilet starboardContour = makeParallelContour(baseContour, starboardOffset, line_join_style, tolerance);
            r.addContour(starboardContour);

            hilet portContour = makeInverseContour(makeParallelContour(baseContour, portOffset, line_join_style, tolerance));
            r.addContour(portContour);
        }

        return r;
    }

    /** Center and scale a path inside the extent with padding.
     */
    [[nodiscard]] graphic_path centerScale(extent2 extent, float padding = 0.0) const noexcept
    {
        auto max_size =
            extent2{std::max(1.0f, extent.width() - (padding * 2.0f)), std::max(1.0f, extent.height() - (padding * 2.0f))};

        auto bbox = boundingBox();
        if (bbox.width() <= 0.0 || bbox.height() <= 0.0) {
            return {};
        }

        hilet scale = std::min(max_size.width() / bbox.width(), max_size.height() / bbox.height());
        bbox = scale2(scale) * bbox;

        hilet offset = (point2{} - get<0>(bbox)) + (extent - bbox.size()) * 0.5;

        return (translate2(offset) * scale2(scale, scale)) * *this;
    }

    graphic_path& operator+=(graphic_path const& rhs) noexcept
    {
        hi_assert(!isContourOpen());
        hi_assert(!rhs.isContourOpen());

        // Left hand layer can only be open if the right hand side contains no layers.
        hi_assert(!rhs.hasLayers() || !isLayerOpen());

        hilet pointOffset = ssize(points);
        hilet contourOffset = ssize(contourEndPoints);

        layerEndContours.reserve(layerEndContours.size() + rhs.layerEndContours.size());
        for (hilet & [ x, fill_color ] : rhs.layerEndContours) {
            layerEndContours.emplace_back(contourOffset + x, fill_color);
        }

        contourEndPoints.reserve(contourEndPoints.size() + rhs.contourEndPoints.size());
        for (hilet x : rhs.contourEndPoints) {
            contourEndPoints.push_back(pointOffset + x);
        }

        points.insert(points.end(), rhs.points.begin(), rhs.points.end());
        return *this;
    }

    [[nodiscard]] friend graphic_path operator+(graphic_path lhs, graphic_path const& rhs) noexcept
    {
        return lhs += rhs;
    }

    friend graphic_path operator*(transformer2 auto const& lhs, graphic_path const& rhs) noexcept
    {
        auto rhs_ = rhs;
        for (auto& point : rhs_.points) {
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
export void composit(pixmap_span<sfloat_rgba16> dst, hi::color color, graphic_path const& mask) noexcept
{
    hi_assert(not mask.hasLayers());
    hi_assert(not mask.isContourOpen());

    auto mask_image = pixmap<uint8_t>(dst.width(), dst.height());
    fill(mask_image);

    hilet curves = mask.getBeziers();
    fill(mask_image, curves);

    composit(dst, color, mask_image);
}

/** Composit color onto the destination image where the mask is solid.
 *
 * \param dst destination image.
 * \param mask mask where the color will be composited on the destination.
 */
export void composit(pixmap_span<sfloat_rgba16> dst, graphic_path const& mask) noexcept
{
    hi_assert(mask.hasLayers() and not mask.isLayerOpen());

    for (int layerNr = 0; layerNr < mask.numberOfLayers(); layerNr++) {
        hilet[layer, fill_color] = mask.getLayer(layerNr);

        composit(dst, fill_color, layer);
    }
}

/** Fill a signed distance field image from the given path.
 * @param dst An signed-distance-field which show distance toward the closest curve
 * @param path A path.
 */
export void fill(pixmap_span<sdf_r8> dst, graphic_path const& path) noexcept
{
    fill(dst, path.getBeziers());
}

}} // namespace hi::v1
