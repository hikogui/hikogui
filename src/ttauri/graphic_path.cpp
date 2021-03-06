// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "graphic_path.hpp"
#include "pixel_map.inl"
#include "bezier_curve.hpp"
#include "pixel_map.hpp"
#include "required.hpp"

namespace tt {

ssize_t graphic_path::numberOfContours() const noexcept
{
    return std::ssize(contourEndPoints);
}

ssize_t graphic_path::numberOfLayers() const noexcept
{
    return std::ssize(layerEndContours);
}

bool graphic_path::hasLayers() const noexcept
{
    return numberOfLayers() > 0;
}

bool graphic_path::allLayersHaveSameColor() const noexcept
{
    if (!hasLayers()) {
        return true;
    }

    ttlet &firstColor = layerEndContours.front().second;

    for (ttlet &[endContour, color] : layerEndContours) {
        if (color != firstColor) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] aarectangle graphic_path::boundingBox() const noexcept
{
    if (std::ssize(points) == 0) {
        return aarectangle{0.0, 0.0, 0.0, 0.0};
    }

    auto r = aarectangle{points.front().p, points.front().p};

    for (ttlet &point: points) {
        r |= point.p;
    }

    return r;
}

void graphic_path::tryRemoveLayers() noexcept
{
    if (!hasLayers()) {
        return;
    }

    if (!allLayersHaveSameColor()) {
        return;
    }

    layerEndContours.clear();
}

std::vector<bezier_point>::const_iterator graphic_path::beginContour(ssize_t contourNr) const noexcept
{
    return points.begin() + (contourNr == 0 ? 0 : contourEndPoints.at(contourNr - 1) + 1);
}

std::vector<bezier_point>::const_iterator graphic_path::endContour(ssize_t contourNr) const noexcept
{
    return points.begin() + contourEndPoints.at(contourNr) + 1;
}

ssize_t graphic_path::beginLayer(ssize_t layerNr) const noexcept
{
    return layerNr == 0 ? 0 : layerEndContours.at(layerNr - 1).first + 1;
}

ssize_t graphic_path::endLayer(ssize_t layerNr) const noexcept
{
    return layerEndContours.at(layerNr).first + 1;
}

color graphic_path::getColorOfLayer(ssize_t layerNr) const noexcept
{
    return layerEndContours.at(layerNr).second;
}

void graphic_path::setColorOfLayer(ssize_t layerNr, color fillColor) noexcept
{
    layerEndContours.at(layerNr).second = fillColor;
}

std::pair<graphic_path,color> graphic_path::getLayer(ssize_t layerNr) const noexcept
{
    tt_assert(hasLayers());

    auto path = graphic_path{};

    ttlet begin = beginLayer(layerNr);
    ttlet end = endLayer(layerNr);
    for (ssize_t contourNr = begin; contourNr != end; contourNr++) {
        path.addContour(beginContour(contourNr), endContour(contourNr));
    }

    return {path, getColorOfLayer(layerNr)};
}

void graphic_path::optimizeLayers() noexcept
{
    if (std::ssize(layerEndContours) == 0) {
        return;
    }

    decltype(layerEndContours) tmp;
    tmp.reserve(std::ssize(layerEndContours));

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


std::vector<bezier_point> graphic_path::getbezier_pointsOfContour(ssize_t subpathNr) const noexcept
{
    ttlet begin = points.begin() + (subpathNr == 0 ? 0 : contourEndPoints.at(subpathNr - 1) + 1);
    ttlet end = points.begin() + contourEndPoints.at(subpathNr) + 1;
    return std::vector(begin, end);
}

std::vector<bezier_curve> graphic_path::getBeziersOfContour(ssize_t contourNr) const noexcept
{
    return makeContourFromPoints(beginContour(contourNr), endContour(contourNr));
}

std::vector<bezier_curve> graphic_path::getBeziers() const noexcept
{
    tt_assert(!hasLayers());

    std::vector<bezier_curve> r;

    for (auto contourNr = 0; contourNr < numberOfContours(); contourNr++) {
        ttlet beziers = getBeziersOfContour(contourNr);
        r.insert(r.end(), beziers.begin(), beziers.end());
    }
    return r;
}

bool graphic_path::isContourOpen() const noexcept
{
    if (points.size() == 0) {
        return false;
    } else if (contourEndPoints.size() == 0) {
        return true;
    } else {
        return contourEndPoints.back() != (std::ssize(points) - 1);
    }
}

void graphic_path::closeContour() noexcept
{
    if (isContourOpen()) {
        contourEndPoints.push_back(std::ssize(points) - 1);
    }
}

bool graphic_path::isLayerOpen() const noexcept
{
    if (points.size() == 0) {
        return false;
    } else if (isContourOpen()) {
        return true;
    } else if (layerEndContours.size() == 0) {
        return true;
    } else {
        return layerEndContours.back().first != (std::ssize(contourEndPoints) - 1);
    }
}

void graphic_path::closeLayer(color fillColor) noexcept
{
    closeContour();
    if (isLayerOpen()) {
        layerEndContours.emplace_back(std::ssize(contourEndPoints) - 1, fillColor);
    }
}

point2 graphic_path::currentPosition() const noexcept
{
    if (isContourOpen()) {
        return points.back().p;
    } else {
        return point2{};
    }
}

void graphic_path::moveTo(point2 position) noexcept
{
    closeContour();
    points.emplace_back(position, bezier_point::Type::Anchor);
}

void graphic_path::moveRelativeTo(vector2 direction) noexcept
{
    tt_assert(isContourOpen());

    ttlet lastPosition = currentPosition();
    closeContour();
    points.emplace_back(lastPosition + direction, bezier_point::Type::Anchor);
}

void graphic_path::lineTo(point2 position) noexcept
{
    tt_assert(isContourOpen());

    points.emplace_back(position, bezier_point::Type::Anchor);
}

void graphic_path::lineRelativeTo(vector2 direction) noexcept
{
    tt_assert(isContourOpen());

    points.emplace_back(currentPosition() + direction, bezier_point::Type::Anchor);
}

void graphic_path::quadraticCurveTo(point2 controlPosition, point2 position) noexcept
{
    tt_assert(isContourOpen());

    points.emplace_back(controlPosition, bezier_point::Type::QuadraticControl);
    points.emplace_back(position, bezier_point::Type::Anchor);
}

void graphic_path::quadraticCurveRelativeTo(vector2 controlDirection, vector2 direction) noexcept
{
    tt_assert(isContourOpen());

    ttlet p = currentPosition();
    points.emplace_back(p + controlDirection, bezier_point::Type::QuadraticControl);
    points.emplace_back(p + direction, bezier_point::Type::Anchor);
}

void graphic_path::cubicCurveTo(point2 controlPosition1, point2 controlPosition2, point2 position) noexcept
{
    tt_assert(isContourOpen());

    points.emplace_back(controlPosition1, bezier_point::Type::CubicControl1);
    points.emplace_back(controlPosition2, bezier_point::Type::CubicControl2);
    points.emplace_back(position, bezier_point::Type::Anchor);
}

void graphic_path::cubicCurveRelativeTo(vector2 controlDirection1, vector2 controlDirection2, vector2 direction) noexcept
{
    tt_assert(isContourOpen());

    ttlet p = currentPosition();
    points.emplace_back(p + controlDirection1, bezier_point::Type::CubicControl1);
    points.emplace_back(p + controlDirection2, bezier_point::Type::CubicControl2);
    points.emplace_back(p + direction, bezier_point::Type::Anchor);
}

void graphic_path::arcTo(float radius, point2 position) noexcept
{
    tt_assert(isContourOpen());

    ttlet r = std::abs(radius);
    ttlet P1 = currentPosition();
    ttlet P2 = position;
    ttlet Pm = midpoint(P1, P2);

    ttlet Vm2 = P2 - Pm;

    // Calculate the half angle between vectors P0 - C and P2 - C.
    ttlet alpha = std::asin(hypot(Vm2) / r);

    // Calculate the center point C. As the length of the normal of Vm2 at Pm.
    ttlet C = Pm + normal(Vm2) * std::cos(alpha) * radius;

    // Calculate vectors from center to end points.
    ttlet VC1 = P1 - C;
    ttlet VC2 = P2 - C;

    ttlet q1 = squared_hypot(VC1);
    ttlet q2 = q1 + dot(VC1, VC2);
    ttlet k2 = (4.0f / 3.0f) * (std::sqrt(2.0f * q1 * q2) - q2) / cross(VC1, VC2);

    // Calculate the control points.
    ttlet C1 = point2{
        (C.x() + VC1.x()) - k2 * VC1.y(),
        (C.y() + VC1.y()) + k2 * VC1.x()
    };
    ttlet C2 = point2{
        (C.x() + VC2.x()) + k2 * VC2.y(),
        (C.y() + VC2.y()) - k2 * VC2.x()
    };

    cubicCurveTo(C1, C2, P2);
}

void graphic_path::addRectangle(aarectangle r, corner_shapes corners) noexcept
{
    tt_assert(!isContourOpen());

    ttlet bl_radius = std::abs(corners.left_bottom());
    ttlet br_radius = std::abs(corners.right_bottom());
    ttlet tl_radius = std::abs(corners.left_top());
    ttlet tr_radius = std::abs(corners.right_top());

    ttlet blc = get<0>(r);
    ttlet brc = get<1>(r);
    ttlet tlc = get<2>(r);
    ttlet trc = get<3>(r);

    ttlet blc1 = blc + vector2{0.0f, bl_radius};
    ttlet blc2 = blc + vector2{bl_radius, 0.0f};
    ttlet brc1 = brc + vector2{-br_radius, 0.0f};
    ttlet brc2 = brc + vector2{0.0f, br_radius};
    ttlet tlc1 = tlc + vector2{tl_radius, 0.0f};
    ttlet tlc2 = tlc + vector2{0.0f, -tl_radius};
    ttlet trc1 = trc + vector2{0.0f, -tr_radius};
    ttlet trc2 = trc + vector2{-tr_radius, 0.0f};

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

void graphic_path::addCircle(point2 position, float radius) noexcept
{
    tt_assert(!isContourOpen());

    moveTo(point2{position.x(), position.y() - radius});
    arcTo(radius, point2{position.x() + radius, position.y()});
    arcTo(radius, point2{position.x(), position.y() + radius});
    arcTo(radius, point2{position.x() - radius, position.y()});
    arcTo(radius, point2{position.x(), position.y() - radius});
    closeContour();
}

void graphic_path::addContour(std::vector<bezier_point>::const_iterator const &begin, std::vector<bezier_point>::const_iterator const &end) noexcept
{
    tt_assert(!isContourOpen());
    points.insert(points.end(), begin, end);
    closeContour();
}

void graphic_path::addContour(std::vector<bezier_point> const &contour) noexcept
{
    addContour(contour.begin(), contour.end());
}

void graphic_path::addContour(std::vector<bezier_curve> const &contour) noexcept
{
    tt_assert(!isContourOpen());

    for (ttlet &curve: contour) {
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
            tt_no_default();
        }
    }

    closeContour();
}

void graphic_path::addPath(graphic_path const &path, color fillColor) noexcept
{
    *this += path;
    closeLayer(fillColor);
}

void graphic_path::addStroke(graphic_path const &path, color strokeColor, float strokeWidth, LineJoinStyle lineJoinStyle, float tolerance) noexcept
{
    *this += path.toStroke(strokeWidth, lineJoinStyle, tolerance);
    closeLayer(strokeColor);
}

graphic_path graphic_path::toStroke(float strokeWidth, LineJoinStyle lineJoinStyle, float tolerance) const noexcept
{
    tt_assert(!hasLayers());
    tt_assert(!isContourOpen());

    auto r = graphic_path{};

    float starboardOffset = strokeWidth / 2;
    float portOffset = -starboardOffset;

    for (int i = 0; i < numberOfContours(); i++) {
        ttlet baseContour = getBeziersOfContour(i);

        ttlet starboardContour = makeParrallelContour(baseContour, starboardOffset, lineJoinStyle, tolerance);
        r.addContour(starboardContour);

        ttlet portContour = makeInverseContour(makeParrallelContour(baseContour, portOffset, lineJoinStyle, tolerance));
        r.addContour(portContour);
    }

    return r;
}

graphic_path &graphic_path::operator+=(graphic_path const &rhs) noexcept
{
    tt_assert(!isContourOpen());
    tt_assert(!rhs.isContourOpen());

    // Left hand layer can only be open if the right hand side contains no layers.
    tt_assert(!rhs.hasLayers() || !isLayerOpen());

    ttlet pointOffset = std::ssize(points);
    ttlet contourOffset = std::ssize(contourEndPoints);

    layerEndContours.reserve(layerEndContours.size() + rhs.layerEndContours.size());
    for (ttlet &[x, fillColor]: rhs.layerEndContours) {
        layerEndContours.emplace_back(contourOffset + x, fillColor);
    }

    contourEndPoints.reserve(contourEndPoints.size() + rhs.contourEndPoints.size());
    for (ttlet x: rhs.contourEndPoints) {
        contourEndPoints.push_back(pointOffset + x);
    }

    points.insert(points.end(), rhs.points.begin(), rhs.points.end());
    return *this;
}

graphic_path graphic_path::centerScale(extent2 extent, float padding) const noexcept
{
    auto max_size = extent2{
        std::max(1.0f, extent.width() - (padding * 2.0f)),
        std::max(1.0f, extent.height() - (padding * 2.0f))
    };

    auto bbox = boundingBox();
    if (bbox.width() <= 0.0 || bbox.height() <= 0.0) {
        return {};
    }

    ttlet scale = std::min(
        max_size.width() / bbox.width(),
        max_size.height() / bbox.height()
    );
    bbox = scale2(scale) * bbox;
    
    ttlet offset = (point2{} - get<0>(bbox)) + (extent - bbox.extent()) * 0.5;

    return (translate2(offset) * scale2(scale, scale)) * *this;
}

void composit(pixel_map<sfloat_rgba16>& dst, color color, graphic_path const &path) noexcept
{
    tt_assert(!path.hasLayers());
    tt_assert(!path.isContourOpen());

    auto mask = pixel_map<uint8_t>(dst.width(), dst.height());
    fill(mask);

    ttlet curves = path.getBeziers();
    fill(mask, curves);

    composit(dst, color, mask);
}

void composit(pixel_map<sfloat_rgba16>& dst, graphic_path const &src) noexcept
{
    tt_assert(src.hasLayers() && !src.isLayerOpen());

    for (int layerNr = 0; layerNr < src.numberOfLayers(); layerNr++) {
        ttlet [layer, fillColor] = src.getLayer(layerNr);

        composit(dst, fillColor, layer);
    }
}

void fill(pixel_map<sdf_r8> &dst, graphic_path const &path) noexcept
{
    fill(dst, path.getBeziers());
}

}
