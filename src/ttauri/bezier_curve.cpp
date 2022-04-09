// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "bezier_curve.hpp"
#include "bezier_point.hpp"
#include "pixel_map.inl"
#include "memory.hpp"
#include <optional>

namespace tt::inline v1 {

static constexpr bezier_curve::Color operator++(bezier_curve::Color& lhs, int) noexcept
{
    auto tmp = lhs;
    lhs = (lhs == bezier_curve::Color::Cyan) ? bezier_curve::Color::Magenta : bezier_curve::Color::Cyan;
    return tmp;
}

std::vector<bezier_curve>
makeContourFromPoints(std::vector<bezier_point>::const_iterator begin, std::vector<bezier_point>::const_iterator end) noexcept
{
    ttlet points = bezier_point::normalizePoints(begin, end);

    std::vector<bezier_curve> r;

    auto type = bezier_curve::Type::None;
    auto P1 = point2{};
    auto C1 = point2{};
    auto C2 = point2{};

    auto color = bezier_curve::Color::Yellow;
    for (ttlet& point : points) {
        switch (point.type) {
        case bezier_point::Type::Anchor:
            switch (type) {
            case bezier_curve::Type::None:
                P1 = point.p;
                type = bezier_curve::Type::Linear;
                break;
            case bezier_curve::Type::Linear:
                r.emplace_back(P1, point.p, color++);
                P1 = point.p;
                type = bezier_curve::Type::Linear;
                break;
            case bezier_curve::Type::Quadratic:
                r.emplace_back(P1, C1, point.p, color++);
                P1 = point.p;
                type = bezier_curve::Type::Linear;
                break;
            case bezier_curve::Type::Cubic:
                r.emplace_back(P1, C1, C2, point.p, color++);
                P1 = point.p;
                type = bezier_curve::Type::Linear;
                break;
            default: tt_no_default();
            }
            break;
        case bezier_point::Type::QuadraticControl:
            C1 = point.p;
            type = bezier_curve::Type::Quadratic;
            break;
        case bezier_point::Type::CubicControl1:
            C1 = point.p;
            type = bezier_curve::Type::Cubic;
            break;
        case bezier_point::Type::CubicControl2:
            C2 = point.p;
            tt_assert(type == bezier_curve::Type::Cubic);
            break;
        default: tt_no_default();
        }
    }

    // If there is only a single curve, water-drop-shaped, it should be marked white.
    if (ssize(r) == 1) {
        r.front().color = bezier_curve::Color::White;
    }

    return r;
}

std::vector<bezier_curve> makeInverseContour(std::vector<bezier_curve> const& contour) noexcept
{
    auto r = std::vector<bezier_curve>{};
    r.reserve(contour.size());

    for (auto i = contour.rbegin(); i != contour.rend(); i++) {
        r.push_back(~(*i));
    }

    return r;
}

std::vector<bezier_curve> makeParallelContour(
    std::vector<bezier_curve> const& contour,
    float offset,
    line_join_style line_join_style,
    float tolerance) noexcept
{
    auto contourAtOffset = std::vector<bezier_curve>{};
    for (ttlet& curve : contour) {
        for (ttlet& flatCurve : curve.subdivideUntilFlat(tolerance)) {
            contourAtOffset.push_back(flatCurve.toParallelLine(offset));
        }
    }

    // The resulting path now consists purely of line-segments that may have gaps and overlaps.
    // This needs to be repaired.
    std::optional<point2> intersectPoint;
    auto r = std::vector<bezier_curve>{};
    for (ttlet& curve : contourAtOffset) {
        if (r.size() == 0) {
            r.push_back(curve);

        } else if (r.back().P2 == curve.P1) {
            r.push_back(curve);

        } else if ((intersectPoint = getIntersectionPoint(r.back().P1, r.back().P2, curve.P1, curve.P2))) {
            r.back().P2 = intersectPoint.value();
            r.push_back(curve);
            r.back().P1 = intersectPoint.value();

        } else if (
            line_join_style == line_join_style::miter &&
            (intersectPoint = getExtrapolatedIntersectionPoint(r.back().P1, r.back().P2, curve.P1, curve.P2))) {
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

static std::vector<float> solveCurvesXByY(std::vector<bezier_curve> const& v, float y) noexcept
{
    std::vector<float> r;
    r.reserve(v.size());

    for (ttlet& curve : v) {
        ttlet xValues = curve.solveXByY(y);
        for (ttlet x : xValues) {
            r.push_back(x);
        }
    }
    return r;
}

static std::optional<std::vector<std::pair<float, float>>> getFillSpansAtY(std::vector<bezier_curve> const& v, float y) noexcept
{
    auto xValues = solveCurvesXByY(v, y);

    // Sort x values, each pair is a span.
    std::sort(xValues.begin(), xValues.end());

    // End-to-end connected curves will yield duplicate values.
    ttlet uniqueEnd = std::unique(xValues.begin(), xValues.end());

    // After removing duplicates, we should end up with pairs of x values.
    std::size_t const uniqueValueCount = (uniqueEnd - xValues.begin());

    if (uniqueValueCount % 2 != 0) {
        // Something is wrong in solving the curves. Probably numeric instability.
        // In any case, just ignore this sample.
        return {};
    }

    // Create pairs of values.
    auto r = std::vector<std::pair<float, float>>{};
    r.reserve(uniqueValueCount / 2);
    for (std::size_t i = 0; i < uniqueValueCount; i += 2) {
        r.emplace_back(xValues[i], xValues[i + 1]);
    }
    return r;
}

static void fillPartialPixels(pixel_row<uint8_t> row, ssize_t const i, float const startX, float const endX) noexcept
{
    ttlet pixelCoverage = std::clamp(endX, i + 0.0f, i + 1.0f) - std::clamp(startX, i + 0.0f, i + 1.0f);

    auto& pixel = row[i];
    pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
}

static void fillFullPixels(pixel_row<uint8_t> row, ssize_t const start, ssize_t const size) noexcept
{
    if (size < 16) {
        ttlet end = start + size;
        for (ssize_t i = start; i < end; i++) {
            row[i] += 0x33;
        }
    } else {
        auto u8p = &row[start];
        ttlet u8end = u8p + size;

        // First add 51 to all pixels up to the alignment.
        ttlet alignedStart = tt::ceil(u8p, sizeof(uint64_t));
        while (u8p < alignedStart) {
            *(u8p++) += 0x33;
        }

        // add 51 for each pixel, 8 pixels at a time.
        auto u64p = reinterpret_cast<uint64_t *>(u8p);
        ttlet u64end = reinterpret_cast<uint64_t *>(tt::floor(u8end, sizeof(uint64_t)));
        while (u64p < u64end) {
            *(u64p++) += 0x3333333333333333ULL;
        }

        // Add 51 to the last pixels.
        u8p = reinterpret_cast<uint8_t *>(u64p);
        while (u8p < u8end) {
            *(u8p++) += 0x33;
        }
    }
}

/*! Render pixels in a row between two x values.
 * Fully covered sub-pixel will have the value 51.
 */
static void fillRowSpan(pixel_row<uint8_t> row, float const startX, float const endX) noexcept
{
    if (startX >= row.width() || endX < 0.0f) {
        return;
    }

    ttlet startX_int = narrow_cast<std::size_t>(startX);
    ttlet endXplusOne = endX + 1.0f;
    ttlet endX_int = narrow_cast<std::size_t>(endXplusOne);
    ttlet startColumn = std::max(startX_int, std::size_t{0});
    ttlet endColumn = std::min(endX_int, row.width());
    ttlet nrColumns = endColumn - startColumn;

    if (nrColumns == 1) {
        fillPartialPixels(row, startColumn, startX, endX);
    } else {
        fillPartialPixels(row, startColumn, startX, endX);
        fillFullPixels(row, startColumn + 1, nrColumns - 2);
        fillPartialPixels(row, endColumn - 1, startX, endX);
    }
}

static void fillRow(pixel_row<uint8_t> row, std::size_t rowY, std::vector<bezier_curve> const& curves) noexcept
{
    // 5 times super sampling.
    for (float y = rowY + 0.1f; y < (rowY + 1); y += 0.2f) {
        auto optionalSpans = getFillSpansAtY(curves, y);
        if (!optionalSpans) {
            // try again, with a slight offset.
            optionalSpans = getFillSpansAtY(curves, y + 0.01f);
        }

        if (optionalSpans) {
            ttlet& spans = optionalSpans.value();

            for (ttlet& span : spans) {
                fillRowSpan(row, span.first, span.second);
            }
        }
    }
}

void fill(pixel_map<uint8_t>& image, std::vector<bezier_curve> const& curves) noexcept
{
    for (std::size_t rowNr = 0; rowNr < image.height(); rowNr++) {
        fillRow(image.at(rowNr), rowNr, curves);
    }
}

[[nodiscard]] static float generate_sdf_r8_pixel(point2 point, std::vector<bezier_curve> const& curves) noexcept
{
    if (size(curves) == 0) {
        return -std::numeric_limits<float>::max();
    }

    float nearest_sq_distance = std::numeric_limits<float>::max();
    float nearest_orthogonality = std::numeric_limits<float>::max();
    for (ttlet& curve : curves) {
        ttlet[sq_distance, orthogonality] = curve.sdf_squared_distance(point);

        if (abs(sq_distance - nearest_sq_distance) < 0.0001f) {
            // If the shortest distance is to a corner, use the edge that is closer.
            if (abs(orthogonality) > abs(nearest_orthogonality)) {
                nearest_sq_distance = sq_distance;
                nearest_orthogonality = orthogonality;
            }
        } else if (sq_distance < nearest_sq_distance) {
            // If it is not a corner and this edge is closer.
            nearest_sq_distance = sq_distance;
            nearest_orthogonality = orthogonality;
        }
    }

    ttlet distance = std::sqrt(nearest_sq_distance);
    return nearest_orthogonality < 0.0f ? distance : -distance;
}


void fill(pixel_map<sdf_r8>& image, std::vector<bezier_curve> const& curves) noexcept
{
    for (int row_nr = 0; row_nr != image.height(); ++row_nr) {
        auto row = image.at(row_nr);
        auto y = static_cast<float>(row_nr);
        for (int column_nr = 0; column_nr != image.width(); ++column_nr) {
            auto x = static_cast<float>(column_nr);
            row[column_nr] = generate_sdf_r8_pixel(point2(x, y), curves);
        }
    }
}

} // namespace tt::inline v1
