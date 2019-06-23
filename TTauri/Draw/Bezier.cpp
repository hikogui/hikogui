// Copyright 2019 Pokitec
// All rights reserved.

#include "Bezier.hpp"
#include "BezierPoint.hpp"
#include "PixelMap.inl"
#include "TTauri/utils.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace TTauri::Draw {

std::vector<Bezier> makeContourFromPoints(std::vector<BezierPoint> const& _points)
{
    let points = BezierPoint::normalizePoints(_points);

    std::vector<Bezier> r;

    auto type = Bezier::Type::None;
    auto P1 = glm::vec2{};
    auto C1 = glm::vec2{};
    auto C2 = glm::vec2{};

    for (let &point: points) {
        switch (point.type) {
        case BezierPoint::Type::Anchor:
            switch (type) {
            case Bezier::Type::None:
                P1 = point.p;
                type = Bezier::Type::Linear;
                break;
            case Bezier::Type::Linear:
                r.emplace_back(P1, point.p);
                P1 = point.p;
                type = Bezier::Type::Linear;
                break;
            case Bezier::Type::Quadratic:
                r.emplace_back(P1, C1, point.p);
                P1 = point.p;
                type = Bezier::Type::Linear;
                break;
            case Bezier::Type::Cubic:
                r.emplace_back(P1, C1, C2, point.p);
                P1 = point.p;
                type = Bezier::Type::Linear;
                break;
            default:
                no_default;
            }
            break;
        case BezierPoint::Type::QuadraticControl:
            C1 = point.p;
            type = Bezier::Type::Quadratic;
            break;
        case BezierPoint::Type::CubicControl1:
            C1 = point.p;
            type = Bezier::Type::Cubic;
            break;
        case BezierPoint::Type::CubicControl2:
            C2 = point.p;
            required_assert(type == Bezier::Type::Cubic);
            break;
        default:
            no_default;
        }
    }

    return r;
}

std::vector<Bezier> makeInverseContour(std::vector<Bezier> const &contour)
{
    auto r = std::vector<Bezier>{};
    r.reserve(contour.size());

    for (auto i = contour.rbegin(); i != contour.rend(); i++) {
        r.push_back(~(*i));
    }

    return r;
}

std::vector<Bezier> makeParrallelContour(std::vector<Bezier> const &contour, float offset, LineJoinStyle lineJoinStyle, float tolerance)
{
    auto contourAtOffset = std::vector<Bezier>{};
    for (let &curve: contour) {
        for (let &flatCurve: curve.subdivideUntilFlat(tolerance)) {
            contourAtOffset.push_back(flatCurve.toParrallelLine(offset));
        }
    }

    // The resulting path now consists purely of line-segments that may have gaps and overlaps.
    // This needs to be repaired.
    std::optional<glm::vec2> intersectPoint;
    auto r = std::vector<Bezier>{};
    for (let &curve: contourAtOffset) {
        if (r.size() == 0) {
            r.push_back(curve);

        } else if (r.back().P2 == curve.P1) {
            r.push_back(curve);

        } else if ((intersectPoint = getIntersectionPoint(r.back().P1, r.back().P2, curve.P1, curve.P2))) {
            r.back().P2 = intersectPoint.value();
            r.push_back(curve);
            r.back().P1 = intersectPoint.value();

        } else if (lineJoinStyle == LineJoinStyle::Miter && (intersectPoint = getExtrapolatedIntersectionPoint(r.back().P1, r.back().P2, curve.P1, curve.P2))) {
            r.back().P2 = intersectPoint.value();
            r.push_back(curve);
            r.back().P1 = intersectPoint.value();

        } else {
            r.emplace_back(r.back().P2, curve.P1);
            r.push_back(curve);
        } 
    }

    // Repair the endpoints of the contour as well.
    if (r.size() > 0 && r.back().P2 != r.front().P1) {
        if (let intersectPoint = getIntersectionPoint(r.back().P1, r.back().P2, r.front().P1, r.front().P2)) {
            r.back().P2 = intersectPoint.value();
            r.front().P1 = intersectPoint.value();
        } else {
            r.emplace_back(r.back().P2, r.front().P1);
        }
    }

    return r;
}

static std::vector<float> solveCurvesXByY(std::vector<Bezier> const &v, float y) {
    std::vector<float> r;
    r.reserve(v.size());

    for (let &curve: v) {
        let xValues = curve.solveXByY(y);
        for (let x: xValues) {
            r.push_back(x);
        }
    }
    return r;
}

static std::vector<std::pair<float, float>> getFillSpansAtY(std::vector<Bezier> const &v, float y) {
    auto xValues = solveCurvesXByY(v, y);

    // Sort x values, each pair is a span.
    std::sort(xValues.begin(), xValues.end());

    // End-to-end connected curves will yield duplicate values.
    let uniqueEnd = std::unique(xValues.begin(), xValues.end());

    // After removing duplicates, we should end up with pairs of x values.
    size_t const uniqueValueCount = (uniqueEnd - xValues.begin());
    assert(uniqueValueCount % 2 == 0);

    // Create pairs of values.
    auto r = std::vector<std::pair<float, float>>{};
    r.reserve(uniqueValueCount / 2);
    for (size_t i = 0; i < uniqueValueCount; i += 2) {
        r.emplace_back(xValues[i], xValues[i+1]);
    }
    return r;
}

static void fillPartialPixels(PixelRow<uint8_t> row, size_t i, float const startX, float const endX)
{
    let pixelCoverage =
        std::clamp(endX, i + 0.0f, i + 1.0f) -
        std::clamp(startX, i + 0.0f, i + 1.0f);

    auto & pixel = row[i];
    pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
}

static void fillFullPixels(PixelRow<uint8_t> row, size_t start, size_t size)
{
    if (size < 16) {
        let end = start + size;
        for (size_t i = start; i < end; i++) {
            row[i] += 0x33;
        }
    } else {
        auto u8p = &row[start];
        let u8end = u8p + size;

        // First add 51 to all pixels up to the alignment.
        let alignedStart = TTauri::align<uint8_t*>(u8p, sizeof(uint64_t));
        while (u8p < alignedStart) {
            *(u8p++) += 0x33;
        }

        // add 51 for each pixel, 8 pixels at a time.
        auto u64p = reinterpret_cast<uint64_t*>(u8p);
        let u64end = TTauri::align_end<uint64_t*>(u8end, sizeof(uint64_t));
        while (u64p < u64end) {
            *(u64p++) += 0x3333333333333333ULL;
        }

        // Add 51 to the last pixels.
        u8p = reinterpret_cast<uint8_t*>(u64p);
        while (u8p < u8end) {
            *(u8p++) += 0x33;
        }
    }
}

/*! Render pixels in a row between two x values.
 * Fully covered sub-pixel will have the value 51.
 */
static void fillRowSpan(PixelRow<uint8_t> row, float const startX, float const endX)
{
    if (startX >= row.width || endX < 0.0f) {
        return;
    }

    let startX_int = static_cast<int64_t>(startX);
    let endXplusOne = endX + 1.0f;
    let endX_int = static_cast<int64_t>(endXplusOne);
    let startColumn = std::max(startX_int, static_cast<int64_t>(0));
    let endColumn = std::min(endX_int, static_cast<int64_t>(row.width));
    let nrColumns = endColumn - startColumn;

    if (nrColumns == 1) {
        fillPartialPixels(row, startColumn, startX, endX);
    } else {
        fillPartialPixels(row, startColumn, startX, endX);
        fillFullPixels(row, startColumn + 1, nrColumns - 2);
        fillPartialPixels(row, endColumn - 1, startX, endX);
    }
}

static void fillSubRow(PixelRow<uint8_t> row, float rowY, std::vector<Bezier> const& curves)
{
    let spans = getFillSpansAtY(curves, rowY);

    for (let &span: spans) {
        fillRowSpan(row, span.first, span.second);
    }
}

static void fillRow(PixelRow<uint8_t> row, size_t rowY, std::vector<Bezier> const& curves)
{
    // 5 times super sampling.
    for (float y = rowY + 0.1f; y < (rowY + 1); y += 0.2f) {
        fillSubRow(row, y, curves);
    }
}

void fill(PixelMap<uint8_t> &image, std::vector<Bezier> const &curves)
{
    for (size_t rowNr = 0; rowNr < image.height; rowNr++) {
        fillRow(image.at(rowNr), rowNr, curves);
    }
}

}