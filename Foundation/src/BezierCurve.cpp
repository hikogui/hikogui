// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/BezierCurve.hpp"
#include "TTauri/Foundation/BezierPoint.hpp"
#include "TTauri/Foundation/PixelMap.inl"
#include "TTauri/Foundation/memory.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace TTauri {


std::vector<BezierCurve> makeContourFromPoints(std::vector<BezierPoint>::const_iterator begin, std::vector<BezierPoint>::const_iterator end) noexcept
{
    let points = BezierPoint::normalizePoints(begin, end);

    std::vector<BezierCurve> r;

    auto type = BezierCurve::Type::None;
    auto P1 = glm::vec2{};
    auto C1 = glm::vec2{};
    auto C2 = glm::vec2{};

    for (let &point: points) {
        switch (point.type) {
        case BezierPoint::Type::Anchor:
            switch (type) {
            case BezierCurve::Type::None:
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            case BezierCurve::Type::Linear:
                r.emplace_back(P1, point.p);
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            case BezierCurve::Type::Quadratic:
                r.emplace_back(P1, C1, point.p);
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            case BezierCurve::Type::Cubic:
                r.emplace_back(P1, C1, C2, point.p);
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            default:
                no_default;
            }
            break;
        case BezierPoint::Type::QuadraticControl:
            C1 = point.p;
            type = BezierCurve::Type::Quadratic;
            break;
        case BezierPoint::Type::CubicControl1:
            C1 = point.p;
            type = BezierCurve::Type::Cubic;
            break;
        case BezierPoint::Type::CubicControl2:
            C2 = point.p;
            required_assert(type == BezierCurve::Type::Cubic);
            break;
        default:
            no_default;
        }
    }

    return r;
}


std::vector<BezierCurve> makeInverseContour(std::vector<BezierCurve> const &contour) noexcept
{
    auto r = std::vector<BezierCurve>{};
    r.reserve(contour.size());

    for (auto i = contour.rbegin(); i != contour.rend(); i++) {
        r.push_back(~(*i));
    }

    return r;
}


std::vector<BezierCurve> makeParrallelContour(std::vector<BezierCurve> const &contour, float offset, LineJoinStyle lineJoinStyle, float tolerance) noexcept
{
    auto contourAtOffset = std::vector<BezierCurve>{};
    for (let &curve: contour) {
        for (let &flatCurve: curve.subdivideUntilFlat(tolerance)) {
            contourAtOffset.push_back(flatCurve.toParrallelLine(offset));
        }
    }

    // The resulting path now consists purely of line-segments that may have gaps and overlaps.
    // This needs to be repaired.
    std::optional<glm::vec2> intersectPoint;
    auto r = std::vector<BezierCurve>{};
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
        if ((intersectPoint = getIntersectionPoint(r.back().P1, r.back().P2, r.front().P1, r.front().P2))) {
            r.back().P2 = r.front().P1 = intersectPoint.value();
        } else {
            r.emplace_back(r.back().P2, r.front().P1);
        }
    }

    return r;
}


static std::vector<float> solveCurvesXByY(std::vector<BezierCurve> const &v, float y) noexcept {
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


static std::optional<std::vector<std::pair<float,float>>> getFillSpansAtY(std::vector<BezierCurve> const &v, float y) noexcept
{
    auto xValues = solveCurvesXByY(v, y);

    // Sort x values, each pair is a span.
    std::sort(xValues.begin(), xValues.end());

    // End-to-end connected curves will yield duplicate values.
    let uniqueEnd = std::unique(xValues.begin(), xValues.end());

    // After removing duplicates, we should end up with pairs of x values.
    size_t const uniqueValueCount = (uniqueEnd - xValues.begin());

    if (uniqueValueCount % 2 != 0) {
        // Something is wrong in solving the curves. Probably numeric instability.
        // In any case, just ignore this sample.
        return {};
    }

    // Create pairs of values.
    auto r = std::vector<std::pair<float, float>>{};
    r.reserve(uniqueValueCount / 2);
    for (size_t i = 0; i < uniqueValueCount; i += 2) {
        r.emplace_back(xValues[i], xValues[i+1]);
    }
    return r;
}

static void fillPartialPixels(PixelRow<uint8_t> row, int const i, float const startX, float const endX) noexcept
{
    let pixelCoverage =
        std::clamp(endX, i + 0.0f, i + 1.0f) -
        std::clamp(startX, i + 0.0f, i + 1.0f);

    auto & pixel = row[i];
    pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
}

gsl_suppress2(26489,lifetime.1)
static void fillFullPixels(PixelRow<uint8_t> row, int const start, int const size) noexcept
{
    if (size < 16) {
        let end = start + size;
        for (int i = start; i < end; i++) {
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
static void fillRowSpan(PixelRow<uint8_t> row, float const startX, float const endX) noexcept
{
    if (startX >= row.width || endX < 0.0f) {
        return;
    }

    let startX_int = static_cast<int>(startX);
    let endXplusOne = endX + 1.0f;
    let endX_int = static_cast<int>(endXplusOne);
    let startColumn = std::max(startX_int, 0);
    let endColumn = std::min(endX_int, row.width);
    let nrColumns = endColumn - startColumn;

    if (nrColumns == 1) {
        fillPartialPixels(row, startColumn, startX, endX);
    } else {
        fillPartialPixels(row, startColumn, startX, endX);
        fillFullPixels(row, startColumn + 1, nrColumns - 2);
        fillPartialPixels(row, endColumn - 1, startX, endX);
    }
}

static void fillRow(PixelRow<uint8_t> row, int const rowY, std::vector<BezierCurve> const& curves) noexcept
{
    // 5 times super sampling.
    for (float y = rowY + 0.1f; y < (rowY + 1); y += 0.2f) {
        auto optionalSpans = getFillSpansAtY(curves, y);
        if (!optionalSpans) {
            // try again, with a slight offset.
            optionalSpans = getFillSpansAtY(curves, y + 0.01f);
        }

        if (optionalSpans) {
            let &spans = optionalSpans.value();

            for (let &span: spans) {
                fillRowSpan(row, span.first, span.second);
            }
        }
    }
}

void fill(PixelMap<uint8_t> &image, std::vector<BezierCurve> const &curves) noexcept
{
    for (int rowNr = 0; rowNr < image.height; rowNr++) {
        fillRow(image.at(rowNr), rowNr, curves);
    }
}

}
