// Copyright 2019 Pokitec
// All rights reserved.

#include "Path.hpp"
#include "PixelMap.inl"
#include "BezierCurve.hpp"
#include "PathString.hpp"
#include "Font.hpp"
#include "PixelMap.hpp"
#include "TTauri/Color.hpp"
#include "TTauri/required.hpp"

#include <glm/gtx/matrix_transform_2d.hpp>

namespace TTauri::Draw {

glm::vec2 Path::advanceForGrapheme(size_t index) const
{
    let ligatureRatio = 1.0f / numberOfGraphemes;

    return (advance * ligatureRatio) * static_cast<float>(index);
}

size_t Path::numberOfContours() const
{
    return contourEndPoints.size();
}

size_t Path::numberOfLayers() const
{
    return layerEndContours.size();
}

bool Path::hasLayers() const {
    return numberOfLayers() > 0;
}

std::vector<BezierPoint>::const_iterator Path::beginContour(size_t contourNr) const
{
    return points.begin() + (contourNr == 0 ? 0 : contourEndPoints.at(contourNr - 1) + 1);
}

std::vector<BezierPoint>::const_iterator Path::endContour(size_t contourNr) const
{
    return points.begin() + contourEndPoints.at(contourNr) + 1;
}

size_t Path::beginLayer(size_t layerNr) const
{
    return layerNr == 0 ? 0 : layerEndContours.at(layerNr - 1).first + 1;
}

size_t Path::endLayer(size_t layerNr) const
{
    return layerEndContours.at(layerNr).first + 1;
}

wsRGBA Path::getColorOfLayer(size_t layerNr) const
{
    return layerEndContours.at(layerNr).second;
}

void Path::setColorOfLayer(size_t layerNr, wsRGBA fillColor)
{
    layerEndContours.at(layerNr).second = fillColor;
}

std::pair<Path,wsRGBA> Path::getLayer(size_t layerNr) const
{
    required_assert(hasLayers());

    auto path = Path{};

    let begin = beginLayer(layerNr);
    let end = endLayer(layerNr);
    for (size_t contourNr = begin; contourNr != end; contourNr++) {
        path.addContour(beginContour(contourNr), endContour(contourNr));
    }

    return {path, getColorOfLayer(layerNr)};
}

std::vector<BezierPoint> Path::getBezierPointsOfContour(size_t subpathNr) const
{
    let begin = points.begin() + (subpathNr == 0 ? 0 : contourEndPoints.at(subpathNr - 1) + 1);
    let end = points.begin() + contourEndPoints.at(subpathNr) + 1;
    return std::vector(begin, end);
}

std::vector<BezierCurve> Path::getBeziersOfContour(size_t contourNr) const
{
    return makeContourFromPoints(beginContour(contourNr), endContour(contourNr));
}

std::vector<BezierCurve> Path::getBeziers() const
{
    required_assert(!hasLayers());

    std::vector<BezierCurve> r;

    for (auto contourNr = 0; contourNr < numberOfContours(); contourNr++) {
        let beziers = getBeziersOfContour(contourNr);
        r.insert(r.end(), beziers.begin(), beziers.end());
    }
    return r;
}

bool Path::isContourOpen() const
{
    if (points.size() == 0) {
        return false;
    } else if (contourEndPoints.size() == 0) {
        return true;
    } else {
        return contourEndPoints.back() != (points.size() - 1);
    }
}

void Path::closeContour()
{
    if (isContourOpen()) {
        contourEndPoints.push_back(points.size() - 1);
    }
}

bool Path::isLayerOpen() const
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

void Path::closeLayer(wsRGBA fillColor)
{
    closeContour();
    if (isLayerOpen()) {
        layerEndContours.emplace_back(contourEndPoints.size() - 1, fillColor);
    }
}

glm::vec2 Path::currentPosition() const
{
    if (isContourOpen()) {
        return points.back().p;
    } else {
        return {0.0f, 0.0f};
    }
}

void Path::moveTo(glm::vec2 position)
{
    closeContour();
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::moveRelativeTo(glm::vec2 direction)
{
    required_assert(isContourOpen());

    let lastPosition = currentPosition();
    closeContour();
    points.emplace_back(lastPosition + direction, BezierPoint::Type::Anchor);
}

void Path::lineTo(glm::vec2 position)
{
    required_assert(isContourOpen());
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::lineRelativeTo(glm::vec2 direction)
{
    required_assert(isContourOpen());
    points.emplace_back(currentPosition() + direction, BezierPoint::Type::Anchor);
}

void Path::quadraticCurveTo(glm::vec2 controlPosition, glm::vec2 position)
{
    required_assert(isContourOpen());
    points.emplace_back(controlPosition, BezierPoint::Type::QuadraticControl);
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::quadraticCurveRelativeTo(glm::vec2 controlDirection, glm::vec2 direction)
{
    required_assert(isContourOpen());
    let p = currentPosition();
    points.emplace_back(p + controlDirection, BezierPoint::Type::QuadraticControl);
    points.emplace_back(p + direction, BezierPoint::Type::Anchor);
}

void Path::cubicCurveTo(glm::vec2 controlPosition1, glm::vec2 controlPosition2, glm::vec2 position)
{
    required_assert(isContourOpen());
    points.emplace_back(controlPosition1, BezierPoint::Type::CubicControl1);
    points.emplace_back(controlPosition2, BezierPoint::Type::CubicControl2);
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::cubicCurveRelativeTo(glm::vec2 controlDirection1, glm::vec2 controlDirection2, glm::vec2 direction)
{
    required_assert(isContourOpen());
    let p = currentPosition();
    points.emplace_back(p + controlDirection1, BezierPoint::Type::CubicControl1);
    points.emplace_back(p + controlDirection2, BezierPoint::Type::CubicControl2);
    points.emplace_back(p + direction, BezierPoint::Type::Anchor);
}

void Path::arcTo(float radius, glm::vec2 position)
{
    required_assert(isContourOpen());

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

void Path::addRectangle(rect2 rect, glm::vec4 corners)
{
    required_assert(!isContourOpen());

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

void Path::addContour(std::vector<BezierPoint>::const_iterator const &begin, std::vector<BezierPoint>::const_iterator const &end)
{
    required_assert(!isContourOpen());
    points.insert(points.end(), begin, end);
    closeContour();
}

void Path::addContour(std::vector<BezierPoint> const &contour)
{
    addContour(contour.begin(), contour.end());
}

void Path::addContour(std::vector<BezierCurve> const &contour)
{
    required_assert(!isContourOpen());

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

void Path::addPath(Path const &path, wsRGBA fillColor)
{
    *this += path;
    closeLayer(fillColor);
}

void Path::addStroke(Path const &path, wsRGBA strokeColor, float strokeWidth, LineJoinStyle lineJoinStyle, float tolerance)
{
    *this += path.toStroke(strokeWidth, lineJoinStyle, tolerance);
    closeLayer(strokeColor);
}

Path Path::toStroke(float strokeWidth, LineJoinStyle lineJoinStyle, float tolerance) const
{
    required_assert(!hasLayers());
    required_assert(!isContourOpen());

    auto r = Path{};

    float starboardOffset = strokeWidth / 2;
    float portOffset = -starboardOffset;

    for (size_t i = 0; i < numberOfContours(); i++) {
        let baseContour = getBeziersOfContour(i);

        let starboardContour = makeParrallelContour(baseContour, starboardOffset, lineJoinStyle, tolerance);
        r.addContour(starboardContour);

        let portContour = makeInverseContour(makeParrallelContour(baseContour, portOffset, lineJoinStyle, tolerance));
        r.addContour(portContour);
    }

    return r;
}

Path operator+(Path lhs, Path const &rhs)
{
    return lhs += rhs;
}

Path &operator+=(Path &lhs, Path const &rhs)
{
    required_assert(!lhs.isContourOpen());
    required_assert(!rhs.isContourOpen());

    // Left hand layer can only be open if the right hand side contains no layers.
    required_assert(!rhs.hasLayers() || !lhs.isLayerOpen());

    let pointOffset = lhs.points.size();
    let contourOffset = lhs.contourEndPoints.size();

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

Path &operator*=(Path &lhs, glm::mat3x3 const &rhs)
{
    lhs.boundingBox *= rhs;
    lhs.leftSideBearing = (rhs * glm::vec3(lhs.leftSideBearing, 1.0f)).xy;
    lhs.rightSideBearing = (rhs * glm::vec3(lhs.rightSideBearing, 1.0f)).xy;
    lhs.advance = (rhs * glm::vec3(lhs.advance, 0.0f)).xy;
    lhs.ascender = (rhs * glm::vec3(lhs.ascender, 0.0f)).xy;
    lhs.descender = (rhs * glm::vec3(lhs.descender, 0.0f)).xy;
    lhs.capHeight = (rhs * glm::vec3(lhs.capHeight, 0.0f)).xy;
    lhs.xHeight = (rhs * glm::vec3(lhs.xHeight, 0.0f)).xy;

    for (auto &point: lhs.points) {
        point *= rhs;
    }
    return lhs;
}

Path operator*(glm::mat3x3 const &lhs, Path rhs)
{
    return rhs *= lhs;
}

Path operator+(glm::vec2 const &lhs, Path rhs)
{
    return rhs += lhs;
}

Path operator+(Path lhs, glm::vec2 const &rhs)
{
    return lhs += rhs;
}

Path &operator+=(Path &lhs, glm::vec2 const &rhs)
{
    lhs.boundingBox += rhs;
    lhs.leftSideBearing += rhs;
    lhs.rightSideBearing += rhs;

    for (auto &point: lhs.points) {
        point += rhs;
    }
    return lhs;
}


void fill(PixelMap<wsRGBA>& dst, wsRGBA color, Path const &path, SubpixelOrientation subpixelOrientation)
{
    required_assert(!path.hasLayers());
    required_assert(!path.isContourOpen());

    let renderSubpixels = subpixelOrientation != SubpixelOrientation::Unknown;

    auto curves = path.getBeziers();
    if (renderSubpixels) {
        curves = transform<std::vector<BezierCurve>>(curves, [](auto const &curve) {
            return curve * glm::vec2{3.0f, 1.0f};
        });
    }

    auto mask = PixelMap<uint8_t>(renderSubpixels ? dst.width * 3 : dst.width, dst.height);
    fill(mask);
    Draw::fill(mask, curves);

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

void fill(PixelMap<wsRGBA>& dst, Path const &src, SubpixelOrientation subpixelOrientation)
{
    required_assert(src.hasLayers() && !src.isLayerOpen());

    for (size_t layerNr = 0; layerNr < src.numberOfLayers(); layerNr++) {
        let [layer, fillColor] = src.getLayer(layerNr);

        fill(dst, fillColor, layer, subpixelOrientation);
    }
}

}