// Copyright 2019 Pokitec
// All rights reserved.

#include "Path.hpp"
#include "PixelMap.inl"
#include "Bezier.hpp"
#include "Glyphs.hpp"
#include "Font.hpp"
#include "PixelMap.hpp"
#include "TTauri/Color.hpp"
#include "TTauri/required.hpp"

namespace TTauri::Draw {

size_t Path::numberOfContours() const {
    return endPoints.size();
}

std::vector<BezierPoint> Path::getBezierPointsOfContour(size_t subpathNr) const {
    let begin = points.begin() + (subpathNr == 0 ? 0 : endPoints.at(subpathNr - 1) + 1);
    let end = points.begin() + endPoints.at(subpathNr) + 1;
    return std::vector(begin, end);
}

std::vector<Bezier> Path::getBeziersOfContour(size_t subpathNr) const {
    let contourPoints = getBezierPointsOfContour(subpathNr);
    return makeContourFromPoints(contourPoints);
}

std::vector<Bezier> Path::getBeziers() const {
    std::vector<Bezier> r;
    for (size_t subpathNr = 0; subpathNr < numberOfContours(); subpathNr++) {
        for (let bezier: getBeziersOfContour(subpathNr)) {
            r.push_back(std::move(bezier));
        }
    }
    return r;
}

bool Path::hasCurrentPosition() const {
    if (points.size() == 0) {
        return false;
    } else if (endPoints.size() == 0) {
        return true;
    } else {
        return endPoints.back() != (points.size() - 1);
    }
}

glm::vec2 Path::currentPosition() const {
    if (hasCurrentPosition()) {
        return points.back().p;
    } else {
        return {0.0f, 0.0f};
    }
}

void Path::close() {
    if (hasCurrentPosition()) {
        endPoints.push_back(points.size() - 1);
    }
}

void Path::moveTo(glm::vec2 position) {
    close();
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::moveRelativeTo(glm::vec2 direction) {
    let lastPosition = currentPosition();
    close();
    points.emplace_back(lastPosition + direction, BezierPoint::Type::Anchor);
}

void Path::lineTo(glm::vec2 position) {
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::lineRelativeTo(glm::vec2 direction) {
    points.emplace_back(currentPosition() + direction, BezierPoint::Type::Anchor);
}

void Path::quadraticCurveTo(glm::vec2 controlPosition, glm::vec2 position) {
    points.emplace_back(controlPosition, BezierPoint::Type::QuadraticControl);
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::quadraticCurveRelativeTo(glm::vec2 controlDirection, glm::vec2 direction) {
    let p = currentPosition();
    points.emplace_back(p + controlDirection, BezierPoint::Type::QuadraticControl);
    points.emplace_back(p + direction, BezierPoint::Type::Anchor);
}

void Path::cubicCurveTo(glm::vec2 controlPosition1, glm::vec2 controlPosition2, glm::vec2 position) {
    points.emplace_back(controlPosition1, BezierPoint::Type::CubicControl1);
    points.emplace_back(controlPosition2, BezierPoint::Type::CubicControl2);
    points.emplace_back(position, BezierPoint::Type::Anchor);
}

void Path::cubicCurveRelativeTo(glm::vec2 controlDirection1, glm::vec2 controlDirection2, glm::vec2 direction) {
    let p = currentPosition();
    points.emplace_back(p + controlDirection1, BezierPoint::Type::CubicControl1);
    points.emplace_back(p + controlDirection2, BezierPoint::Type::CubicControl2);
    points.emplace_back(p + direction, BezierPoint::Type::Anchor);
}

void Path::arcTo(float radius, glm::vec2 position) {
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

void Path::addRectangle(rect2 rect, glm::vec4 corners) {
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

    close();
}

void Path::addGlyph(Glyph const &glyph, glm::vec2 position, float scale, float rotation) {
    close();
    let currentNrPoints = points.size();
    for (let point: glyph.points) {
        points.push_back(point.transform(position, scale, rotation));
    }
    for (let endPoint: glyph.endPoints) {
        endPoints.push_back(currentNrPoints + endPoint);
    }
}

void Path::addText(std::string const &text, Font const &font, glm::vec2 position, float scale, float rotation, HorizontalAlignment alignment) {
    Glyphs glyphs = font.getGlyphs(text);

    auto glyphPosition = glyphs.getStartPosition(position, scale, rotation, alignment);
    for (size_t i = 0; i < glyphs.size(); i++) {
        let glyph = glyphs.at(i);
        addGlyph(glyph, glyphPosition, scale, rotation);
        glyphPosition += glyphs.glyphAdvanceVector(i, scale, rotation);
    }
}

void Path::addContour(std::vector<Bezier> const &contour) {
    close();

    for (let &curve: contour) {
        // Don't emit the first point, the last point of the contour will wrap around.
        switch (curve.type) {
        case Bezier::Type::Linear:
            points.emplace_back(curve.P2, BezierPoint::Type::Anchor);
            break;
        case Bezier::Type::Quadratic:
            points.emplace_back(curve.C1, BezierPoint::Type::QuadraticControl);
            points.emplace_back(curve.P2, BezierPoint::Type::Anchor);
            break;
        case Bezier::Type::Cubic:
            points.emplace_back(curve.C1, BezierPoint::Type::CubicControl1);
            points.emplace_back(curve.C2, BezierPoint::Type::CubicControl2);
            points.emplace_back(curve.P2, BezierPoint::Type::Anchor);
            break;
        default:
            no_default;
        }
    }

    close();
}

void Path::addPathToStroke(Path const &path, float strokeWidth, LineJoinStyle lineJoinStyle, float tolerance) {
    float starboardOffset = strokeWidth / 2;
    float portOffset = -starboardOffset;

    for (size_t i = 0; i < path.numberOfContours(); i++) {
        let baseContour = path.getBeziersOfContour(i);

        let starboardContour = makeParrallelContour(baseContour, starboardOffset, lineJoinStyle, tolerance);
        addContour(starboardContour);

        let portContour = makeInverseContour(makeParrallelContour(baseContour, portOffset, lineJoinStyle, tolerance));
        addContour(portContour);
    }
}

void fill(PixelMap<wsRGBApm>& dst, wsRGBApm color, Path const &path, SubpixelOrientation subpixelOrientation) {
    let renderSubpixels = subpixelOrientation != SubpixelOrientation::Unknown;

    auto curves = path.getBeziers();
    if (renderSubpixels) {
        curves = transform<std::vector<Bezier>>(curves, [](auto const &curve) {
            return curve * glm::vec2{3.0f, 1.0f};
            });
    }

    auto mask = PixelMap<uint8_t>(renderSubpixels ? dst.width * 3 : dst.width, dst.height);
    clear(mask);
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

void stroke(PixelMap<wsRGBApm>& dst, wsRGBApm color, Path const &path, float strokeWidth, LineJoinStyle lineJoinStyle, SubpixelOrientation subpixelOrientation) {
    Path fillPath{};
    fillPath.addPathToStroke(path, strokeWidth, lineJoinStyle);
    fill(dst, color, fillPath, subpixelOrientation);
}

void stroke(
    PixelMap<wsRGBApm>& dst,
    wsRGBApm color,
    Path const &mask,
    float strokeWidth,
    SubpixelOrientation subpixelOrientation
) {
    return stroke(dst, color, mask, strokeWidth, LineJoinStyle::Miter, subpixelOrientation);
}



}