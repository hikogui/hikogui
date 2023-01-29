// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "bezier_curve.hpp"
#include "bezier_point.hpp"
#include "utility/module.hpp"
#include <optional>

namespace hi::inline v1 {

static constexpr bezier_curve::Color operator++(bezier_curve::Color& lhs, int) noexcept
{
    auto tmp = lhs;
    lhs = (lhs == bezier_curve::Color::Cyan) ? bezier_curve::Color::Magenta : bezier_curve::Color::Cyan;
    return tmp;
}

std::vector<bezier_curve>
makeContourFromPoints(std::vector<bezier_point>::const_iterator begin, std::vector<bezier_point>::const_iterator end) noexcept
{
    hilet points = bezier_point::normalizePoints(begin, end);

    std::vector<bezier_curve> r;

    auto type = bezier_curve::Type::None;
    auto P1 = point2{};
    auto C1 = point2{};
    auto C2 = point2{};

    auto color = bezier_curve::Color::Yellow;
    for (hilet& point : points) {
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
            default: hi_no_default();
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
            hi_assert(type == bezier_curve::Type::Cubic);
            break;
        default: hi_no_default();
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
    for (hilet& curve : contour) {
        for (hilet& flatCurve : curve.subdivideUntilFlat(tolerance)) {
            contourAtOffset.push_back(flatCurve.toParallelLine(offset));
        }
    }

    // The resulting path now consists purely of line-segments that may have gaps and overlaps.
    // This needs to be repaired.
    std::optional<point2> intersectPoint;
    auto r = std::vector<bezier_curve>{};
    for (hilet& curve : contourAtOffset) {
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

    for (hilet& curve : v) {
        hilet xValues = curve.solveXByY(y);
        for (hilet x : xValues) {
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
    hilet uniqueEnd = std::unique(xValues.begin(), xValues.end());

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

static void fillPartialPixels(std::span<uint8_t> row, ssize_t const i, float const startX, float const endX) noexcept
{
    hilet pixelCoverage = std::clamp(endX, i + 0.0f, i + 1.0f) - std::clamp(startX, i + 0.0f, i + 1.0f);

    auto& pixel = row[i];
    pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
}

static void fillFullPixels(std::span<uint8_t> row, ssize_t const start, ssize_t const size) noexcept
{
    if (size < 16) {
        hilet end = start + size;
        for (ssize_t i = start; i < end; i++) {
            row[i] += 0x33;
        }
    } else {
        auto u8p = &row[start];
        hilet u8end = u8p + size;

        // First add 51 to all pixels up to the alignment.
        hilet alignedStart = hi::ceil(u8p, sizeof(uint64_t));
        while (u8p < alignedStart) {
            *(u8p++) += 0x33;
        }

        // add 51 for each pixel, 8 pixels at a time.
        auto u64p = reinterpret_cast<uint64_t *>(u8p);
        auto const * const u64end = reinterpret_cast<uint64_t const *>(hi::floor(u8end, sizeof(uint64_t)));
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
static void fillRowSpan(std::span<uint8_t> row, float const startX, float const endX) noexcept
{
    if (startX >= row.size() || endX < 0.0f) {
        return;
    }

    hilet startX_int = narrow_cast<std::size_t>(startX);
    hilet endXplusOne = endX + 1.0f;
    hilet endX_int = narrow_cast<std::size_t>(endXplusOne);
    hilet startColumn = std::max(startX_int, std::size_t{0});
    hilet endColumn = std::min(endX_int, row.size());
    hilet nrColumns = endColumn - startColumn;

    if (nrColumns == 1) {
        fillPartialPixels(row, startColumn, startX, endX);
    } else {
        fillPartialPixels(row, startColumn, startX, endX);
        fillFullPixels(row, startColumn + 1, nrColumns - 2);
        fillPartialPixels(row, endColumn - 1, startX, endX);
    }
}

static void fillRow(std::span<uint8_t> row, std::size_t rowY, std::vector<bezier_curve> const& curves) noexcept
{
    // 5 times super sampling.
    for (float y = rowY + 0.1f; y < (rowY + 1); y += 0.2f) {
        auto optionalSpans = getFillSpansAtY(curves, y);
        if (!optionalSpans) {
            // try again, with a slight offset.
            optionalSpans = getFillSpansAtY(curves, y + 0.01f);
        }

        if (optionalSpans) {
            hilet& spans = optionalSpans.value();

            for (hilet& span : spans) {
                fillRowSpan(row, span.first, span.second);
            }
        }
    }
}

void fill(pixmap_span<uint8_t> image, std::vector<bezier_curve> const& curves) noexcept
{
    for (auto y = 0_uz; y < image.height(); y++) {
        fillRow(image[y], y, curves);
    }
}

[[nodiscard]] static float generate_sdf_r8_pixel(point2 point, std::vector<bezier_curve> const& curves) noexcept
{
    if (curves.empty()) {
        return -std::numeric_limits<float>::max();
    }

    auto it = curves.cbegin();
    auto nearest = (it++)->sdf_distance(point);

    for (; it != curves.cend(); ++it) {
        hilet distance = it->sdf_distance(point);

        if (distance < nearest) {
            nearest = distance;
        }
    }

    return nearest.signed_distance();
}


void fill(pixmap_span<sdf_r8> image, std::vector<bezier_curve> const& curves) noexcept
{
    for (auto row_nr = 0_uz; row_nr != image.height(); ++row_nr) {
        hilet row = image[row_nr];
        hilet y = static_cast<float>(row_nr);
        for (auto column_nr = 0_uz; column_nr != image.width(); ++column_nr) {
            hilet x = static_cast<float>(column_nr);
            row[column_nr] = generate_sdf_r8_pixel(point2(x, y), curves);
        }
    }
}

} // namespace hi::inline v1
