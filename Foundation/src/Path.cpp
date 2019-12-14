// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/PixelMap.inl"
#include "TTauri/Foundation/BezierCurve.hpp"
#include "TTauri/Foundation/PathString.hpp"
#include "TTauri/Foundation/Font.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/required.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace TTauri {

ssize_t Path::numberOfContours() const noexcept
{
    return to_signed(contourEndPoints.size());
}

ssize_t Path::numberOfLayers() const noexcept
{
    return to_signed(layerEndContours.size());
}

bool Path::hasLayers() const noexcept
{
    return numberOfLayers() > 0;
}

bool Path::allLayersHaveSameColor() const noexcept
{
    if (!hasLayers()) {
        return true;
    }

    let &firstColor = layerEndContours.front().second;

    for (let &[endContour, color] : layerEndContours) {
        if (color != firstColor) {
            return false;
        }
    }
    return true;
}

void Path::tryRemoveLayers() noexcept
{
    if (!hasLayers()) {
        return;
    }

    if (!allLayersHaveSameColor()) {
        return;
    }

    layerEndContours.clear();
}

std::vector<BezierPoint>::const_iterator Path::beginContour(ssize_t contourNr) const noexcept
{
    return points.begin() + (contourNr == 0 ? 0 : contourEndPoints.at(contourNr - 1) + 1);
}

std::vector<BezierPoint>::const_iterator Path::endContour(ssize_t contourNr) const noexcept
{
    return points.begin() + contourEndPoints.at(contourNr) + 1;
}

ssize_t Path::beginLayer(ssize_t layerNr) const noexcept
{
    return layerNr == 0 ? 0 : layerEndContours.at(layerNr - 1).first + 1;
}

ssize_t Path::endLayer(ssize_t layerNr) const noexcept
{
    return layerEndContours.at(layerNr).first + 1;
}

wsRGBA Path::getColorOfLayer(ssize_t layerNr) const noexcept
{
    return layerEndContours.at(layerNr).second;
}

void Path::setColorOfLayer(ssize_t layerNr, wsRGBA fillColor) noexcept
{
    layerEndContours.at(layerNr).second = fillColor;
}

std::pair<Path,wsRGBA> Path::getLayer(ssize_t layerNr) const noexcept
{
    ttauri_assert(hasLayers());

    auto path = Path{};

    let begin = beginLayer(layerNr);
    let end = endLayer(layerNr);
    for (ssize_t contourNr = begin; contourNr != end; contourNr++) {
        path.addContour(beginContour(contourNr), endContour(contourNr));
    }

    return {path, getColorOfLayer(layerNr)};
}

std::vector<BezierPoint> Path::getBezierPointsOfContour(ssize_t subpathNr) const noexcept
{
    let begin = points.begin() + (subpathNr == 0 ? 0 : contourEndPoints.at(subpathNr - 1) + 1);
    let end = points.begin() + contourEndPoints.at(subpathNr) + 1;
    return std::vector(begin, end);
}

std::vector<BezierCurve> Path::getBeziersOfContour(ssize_t contourNr) const noexcept
{
    return makeContourFromPoints(beginContour(contourNr), endContour(contourNr));
}

std::vector<BezierCurve> Path::getBeziers() const noexcept
{
    ttauri_assert(!hasLayers());

    std::vector<BezierCurve> r;

    for (auto contourNr = 0; contourNr < numberOfContours(); contourNr++) {
        let beziers = getBeziersOfContour(contourNr);
        r.insert(r.end(), beziers.begin(), beziers.end());
    }
    return r;
}

bool Path::isContourOpen() const noexcept
{
    if (points.size() == 0) {
        return false;
    } else if (contourEndPoints.size() == 0) {
        return true;
    } else {
        return contourEndPoints.back() != (to_signed(points.size()) - 1);
    }
}

void Path::closeContour() noexcept
{
    if (isContourOpen()) {
        contourEndPoints.push_back(to_signed(points.size()) - 1);
    }
}

bool Path::isLayerOpen() const noexcept
{
    if (points.size() == 0) {
        return false;
    } else if (isContourOpen()) {
        return true;
    } else if (layerEndContours.size() == 0) {
        return true;
    } else {
        return layerEndContours.back().first != (contourEndPoints.size() - 1);
    }
}

void Path::closeLayer(wsRGBA fillColor) noexcept
{
    closeContour();
    if (isLayerOpen()) {
        layerEndContours.emplace_back(to_signed(contourEndPoints.size()) - 1, fillColor);
    }
}

glm::vec2 Path::currentPosition() const noexcept
{
    if (isContourOpen()) {
        return points.back().p;
    } else {
        return {0.0f, 0.0f};
    }
}

void Path::moveTo(glm::vec2 position) noexcept
{
    closeContour();
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::moveRelativeTo(glm::vec2 direction) noexcept
{
    ttauri_assert(isContourOpen());

    let lastPosition = currentPosition();
    closeContour();
    points.emplace_back(lastPosition + direction, BezierPoint::Type::Anchor);
}

void Path::lineTo(glm::vec2 position) noexcept
{
    ttauri_assert(isContourOpen());
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::lineRelativeTo(glm::vec2 direction) noexcept
{
    ttauri_assert(isContourOpen());
    points.emplace_back(currentPosition() + direction, BezierPoint::Type::Anchor);
}

void Path::quadraticCurveTo(glm::vec2 controlPosition, glm::vec2 position) noexcept
{
    ttauri_assert(isContourOpen());
    points.emplace_back(controlPosition, BezierPoint::Type::QuadraticControl);
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::quadraticCurveRelativeTo(glm::vec2 controlDirection, glm::vec2 direction) noexcept
{
    ttauri_assert(isContourOpen());
    let p = currentPosition();
    points.emplace_back(p + controlDirection, BezierPoint::Type::QuadraticControl);
    points.emplace_back(p + direction, BezierPoint::Type::Anchor);
}

void Path::cubicCurveTo(glm::vec2 controlPosition1, glm::vec2 controlPosition2, glm::vec2 position) noexcept
{
    ttauri_assert(isContourOpen());
    points.emplace_back(controlPosition1, BezierPoint::Type::CubicControl1);
    points.emplace_back(controlPosition2, BezierPoint::Type::CubicControl2);
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::cubicCurveRelativeTo(glm::vec2 controlDirection1, glm::vec2 controlDirection2, glm::vec2 direction) noexcept
{
    ttauri_assert(isContourOpen());
    let p = currentPosition();
    points.emplace_back(p + controlDirection1, BezierPoint::Type::CubicControl1);
    points.emplace_back(p + controlDirection2, BezierPoint::Type::CubicControl2);
    points.emplace_back(p + direction, BezierPoint::Type::Anchor);
}

void Path::arcTo(float radius, glm::vec2 position) noexcept
{
    ttauri_assert(isContourOpen());

    let r = std::abs(radius);
    let P1 = currentPosition();
    let P2 = position;
    let Pm = midpoint(P1, P2);

    let Vm2 = P2 - Pm;

    // Calculate the half angle between vectors P0 - C and P2 - C.
    let alpha = std::asin(glm::length(Vm2) / r);

    // Calculate the center point C. As the length of the normal of Vm2 at Pm.
    let C = Pm + normal(Vm2) * std::cos(alpha) * radius;

    // Culate vectors from center to end points.
    let VC1 = P1 - C;
    let VC2 = P2 - C;

    let q1 = VC1.x * VC1.x + VC1.y * VC1.y;
    let q2 = q1 + VC1.x * VC2.x + VC1.y * VC2.y;
    let k2 = (4.0f/3.0f) * (std::sqrt(2.0f * q1 * q2) - q2) / (VC1.x*VC2.y - VC1.y*VC2.x);

    // Calculate the control points.
    let C1 = glm::vec2{
        (C.x + VC1.x) - k2 * VC1.y,
        (C.y + VC1.y) + k2 * VC1.x
    };
    let C2 = glm::vec2{
        (C.x + VC2.x) + k2 * VC2.y,
        (C.y + VC2.y) - k2 * VC2.x
    };

    cubicCurveTo(C1, C2, P2);
}

void Path::addRectangle(rect2 rect, glm::vec4 corners) noexcept
{
    ttauri_assert(!isContourOpen());

    glm::vec4 radii = glm::abs(corners);

    let blc = rect.offset;
    let brc = rect.offset + glm::vec2{rect.extent.x, 0.0f};
    let trc = rect.offset + rect.extent;
    let tlc = rect.offset + glm::vec2{0.0f, rect.extent.y};

    let blc1 = blc + glm::vec2{0.0f, radii.x};
    let blc2 = blc + glm::vec2{radii.x, 0.0f};
    let brc1 = brc + glm::vec2{-radii.y, 0.0f};
    let brc2 = brc + glm::vec2{0.0f, radii.y};
    let trc1 = trc + glm::vec2{0.0f, -radii.z};
    let trc2 = trc + glm::vec2{-radii.z, 0.0f};
    let tlc1 = tlc + glm::vec2{radii.w, 0.0f};
    let tlc2 = tlc + glm::vec2{0.0, -radii.w};

    moveTo(blc1);
    if (corners.x > 0.0) {
        arcTo(radii.x, blc2);
    } else if (corners.x < 0.0) {
        lineTo(blc2);
    }

    lineTo(brc1);
    if (corners.y > 0.0) {
        arcTo(radii.y, brc2);
    } else if (corners.y < 0.0) {
        lineTo(blc2);
    }

    lineTo(trc1);
    if (corners.z > 0.0) {
        arcTo(radii.z, trc2);
    } else if (corners.z < 0.0) {
        lineTo(trc2);
    }

    lineTo(tlc1);
    if (corners.w > 0.0) {
        arcTo(radii.w, tlc2);
    } else if (corners.w < 0.0) {
        lineTo(tlc2);
    }

    closeContour();
}

void Path::addCircle(glm::vec2 position, float radius) noexcept
{
    ttauri_assert(!isContourOpen());

    moveTo({position.x, position.y - radius});
    arcTo(radius, {position.x + radius, position.y});
    arcTo(radius, {position.x, position.y + radius});
    arcTo(radius, {position.x - radius, position.y});
    arcTo(radius, {position.x, position.y - radius});
    closeContour();
}

void Path::addContour(std::vector<BezierPoint>::const_iterator const &begin, std::vector<BezierPoint>::const_iterator const &end) noexcept
{
    ttauri_assert(!isContourOpen());
    points.insert(points.end(), begin, end);
    closeContour();
}

void Path::addContour(std::vector<BezierPoint> const &contour) noexcept
{
    addContour(contour.begin(), contour.end());
}

void Path::addContour(std::vector<BezierCurve> const &contour) noexcept
{
    ttauri_assert(!isContourOpen());

    for (let &curve: contour) {
        // Don't emit the first point, the last point of the contour will wrap around.
        switch (curve.type) {
        case BezierCurve::Type::Linear:
            points.emplace_back(curve.P2, BezierPoint::Type::Anchor);
            break;
        case BezierCurve::Type::Quadratic:
            points.emplace_back(curve.C1, BezierPoint::Type::QuadraticControl);
            points.emplace_back(curve.P2, BezierPoint::Type::Anchor);
            break;
        case BezierCurve::Type::Cubic:
            points.emplace_back(curve.C1, BezierPoint::Type::CubicControl1);
            points.emplace_back(curve.C2, BezierPoint::Type::CubicControl2);
            points.emplace_back(curve.P2, BezierPoint::Type::Anchor);
            break;
        default:
            no_default;
        }
    }

    closeContour();
}

void Path::addPath(Path const &path, wsRGBA fillColor) noexcept
{
    *this += path;
    closeLayer(fillColor);
}

void Path::addStroke(Path const &path, wsRGBA strokeColor, float strokeWidth, LineJoinStyle lineJoinStyle, float tolerance) noexcept
{
    *this += path.toStroke(strokeWidth, lineJoinStyle, tolerance);
    closeLayer(strokeColor);
}

Path Path::toStroke(float strokeWidth, LineJoinStyle lineJoinStyle, float tolerance) const noexcept
{
    ttauri_assert(!hasLayers());
    ttauri_assert(!isContourOpen());

    auto r = Path{};

    float starboardOffset = strokeWidth / 2;
    float portOffset = -starboardOffset;

    for (int i = 0; i < numberOfContours(); i++) {
        let baseContour = getBeziersOfContour(i);

        let starboardContour = makeParrallelContour(baseContour, starboardOffset, lineJoinStyle, tolerance);
        r.addContour(starboardContour);

        let portContour = makeInverseContour(makeParrallelContour(baseContour, portOffset, lineJoinStyle, tolerance));
        r.addContour(portContour);
    }

    return r;
}

Path operator+(Path lhs, Path const &rhs) noexcept
{
    return lhs += rhs;
}

Path &operator+=(Path &lhs, Path const &rhs) noexcept
{
    ttauri_assert(!lhs.isContourOpen());
    ttauri_assert(!rhs.isContourOpen());

    // Left hand layer can only be open if the right hand side contains no layers.
    ttauri_assert(!rhs.hasLayers() || !lhs.isLayerOpen());

    let pointOffset = to_signed(lhs.points.size());
    let contourOffset = to_signed(lhs.contourEndPoints.size());

    lhs.layerEndContours.reserve(lhs.layerEndContours.size() + rhs.layerEndContours.size());
    for (let [x, fillColor]: rhs.layerEndContours) {
        lhs.layerEndContours.emplace_back(contourOffset + x, fillColor);
    }

    lhs.contourEndPoints.reserve(lhs.contourEndPoints.size() + rhs.contourEndPoints.size());
    for (let x: rhs.contourEndPoints) {
        lhs.contourEndPoints.push_back(pointOffset + x);
    }

    lhs.points.insert(lhs.points.end(), rhs.points.begin(), rhs.points.end());
    return lhs;
}

Path &operator*=(Path &lhs, glm::mat3x3 const &rhs) noexcept
{
    lhs.metrics *= rhs;

    for (auto &point: lhs.points) {
        point *= rhs;
    }
    return lhs;
}

Path &operator*=(Path &lhs, float const rhs) noexcept
{
    lhs.metrics *= rhs;

    for (auto &point: lhs.points) {
        point *= rhs;
    }
    return lhs;
}

Path operator*(glm::mat3x3 const &lhs, Path rhs) noexcept
{
    return rhs *= lhs;
}

Path operator*(float const lhs, Path rhs) noexcept
{
    return rhs *= lhs;
}


Path operator+(glm::vec2 const &lhs, Path rhs) noexcept
{
    return rhs += lhs;
}

Path operator+(Path lhs, glm::vec2 const &rhs) noexcept
{
    return lhs += rhs;
}

Path &operator+=(Path &lhs, glm::vec2 const &rhs) noexcept
{
    lhs.metrics += rhs;

    for (auto &point: lhs.points) {
        point += rhs;
    }
    return lhs;
}

void composit(PixelMap<wsRGBA>& dst, wsRGBA color, Path const &path, SubpixelOrientation subpixelOrientation) noexcept
{
    ttauri_assert(!path.hasLayers());
    ttauri_assert(!path.isContourOpen());

    let renderSubpixels = subpixelOrientation != SubpixelOrientation::Unknown;

    auto curves = path.getBeziers();
    if (renderSubpixels) {
        curves = transform<std::vector<BezierCurve>>(curves, [](auto const &curve) {
            return curve * glm::vec2{3.0f, 1.0f};
        });
    }

    auto mask = PixelMap<uint8_t>(renderSubpixels ? dst.width * 3 : dst.width, dst.height);
    fill(mask);
    fill(mask, curves);

    if (renderSubpixels) {
        subpixelFilter(mask);
        if (subpixelOrientation == SubpixelOrientation::RedRight) {
            subpixelFlip(mask);
        }
        subpixelComposit(dst, color, mask);
    } else {
        composit(dst, color, mask);
    }
}

void composit(PixelMap<wsRGBA>& dst, Path const &src, SubpixelOrientation subpixelOrientation) noexcept
{
    ttauri_assert(src.hasLayers() && !src.isLayerOpen());

    for (int layerNr = 0; layerNr < src.numberOfLayers(); layerNr++) {
        let [layer, fillColor] = src.getLayer(layerNr);

        composit(dst, fillColor, layer, subpixelOrientation);
    }
}

}
