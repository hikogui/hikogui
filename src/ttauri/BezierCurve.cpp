// Copyright 2019 Pokitec
// All rights reserved.

#include "BezierCurve.hpp"
#include "BezierPoint.hpp"
#include "PixelMap.inl"
#include "memory.hpp"
#include <optional>

namespace tt {

static constexpr BezierCurve::Color operator++(BezierCurve::Color &lhs, int) noexcept
{
    auto tmp = lhs;
    lhs = (lhs == BezierCurve::Color::Cyan) ? BezierCurve::Color::Magenta : BezierCurve::Color::Cyan;
    return tmp;
}

std::vector<BezierCurve> makeContourFromPoints(std::vector<BezierPoint>::const_iterator begin, std::vector<BezierPoint>::const_iterator end) noexcept
{
    ttlet points = BezierPoint::normalizePoints(begin, end);

    std::vector<BezierCurve> r;

    auto type = BezierCurve::Type::None;
    auto P1 = vec{};
    auto C1 = vec{};
    auto C2 = vec{};

    auto color = BezierCurve::Color::Yellow;
    for (ttlet &point: points) {
        switch (point.type) {
        case BezierPoint::Type::Anchor:
            switch (type) {
            case BezierCurve::Type::None:
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            case BezierCurve::Type::Linear:
                r.emplace_back(P1, point.p, color++);
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            case BezierCurve::Type::Quadratic:
                r.emplace_back(P1, C1, point.p, color++);
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            case BezierCurve::Type::Cubic:
                r.emplace_back(P1, C1, C2, point.p, color++);
                P1 = point.p;
                type = BezierCurve::Type::Linear;
                break;
            default:
                tt_no_default;
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
            tt_assert(type == BezierCurve::Type::Cubic);
            break;
        default:
            tt_no_default;
        }
    }

    // If there is only a single curve, water-drop-shaped, it should be marked white.
    if (std::ssize(r) == 1) {
        r.front().color = BezierCurve::Color::White;
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
    for (ttlet &curve: contour) {
        for (ttlet &flatCurve: curve.subdivideUntilFlat(tolerance)) {
            contourAtOffset.push_back(flatCurve.toParrallelLine(offset));
        }
    }

    // The resulting path now consists purely of line-segments that may have gaps and overlaps.
    // This needs to be repaired.
    std::optional<vec> intersectPoint;
    auto r = std::vector<BezierCurve>{};
    for (ttlet &curve: contourAtOffset) {
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

    for (ttlet &curve: v) {
        ttlet xValues = curve.solveXByY(y);
        for (ttlet x: xValues) {
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
    ttlet uniqueEnd = std::unique(xValues.begin(), xValues.end());

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

static void fillPartialPixels(PixelRow<uint8_t> row, ssize_t const i, float const startX, float const endX) noexcept
{
    ttlet pixelCoverage =
        std::clamp(endX, i + 0.0f, i + 1.0f) -
        std::clamp(startX, i + 0.0f, i + 1.0f);

    auto & pixel = row[i];
    pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
}

static void fillFullPixels(PixelRow<uint8_t> row, ssize_t const start, ssize_t const size) noexcept
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
        ttlet alignedStart = tt::align<uint8_t*>(u8p, sizeof(uint64_t));
        while (u8p < alignedStart) {
            *(u8p++) += 0x33;
        }

        // add 51 for each pixel, 8 pixels at a time.
        auto u64p = reinterpret_cast<uint64_t*>(u8p);
        ttlet u64end = tt::align_end<uint64_t*>(u8end, sizeof(uint64_t));
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

    ttlet startX_int = numeric_cast<ssize_t>(startX);
    ttlet endXplusOne = endX + 1.0f;
    ttlet endX_int = numeric_cast<ssize_t>(endXplusOne);
    ttlet startColumn = std::max(startX_int, ssize_t{0});
    ttlet endColumn = std::min(endX_int, row.width);
    ttlet nrColumns = endColumn - startColumn;

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
            ttlet &spans = optionalSpans.value();

            for (ttlet &span: spans) {
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


 [[nodiscard]] static float generate_SDF8_pixel(vec point, std::vector<BezierCurve> const &curves) noexcept
{
    if (std::ssize(curves) == 0) {
        return -std::numeric_limits<float>::max();
    }

    float min_distance = std::numeric_limits<float>::max();
    for (ttlet &curve: curves) {
        ttlet distance = curve.sdf_distance(point);

        if (std::abs(distance) < std::abs(min_distance)) {
            min_distance = distance;
        }
    }

    return min_distance;
}

static void bad_pixels_edges(PixelMap<SDF8> &image) noexcept
{
    // Bottom edge.
    auto row = image[0];
    for (ssize_t column_nr = 0; column_nr != image.width; ++column_nr) {
        auto &pixel = row[column_nr];
        if (static_cast<float>(pixel) > 0.0) {
            pixel.repair();
        }
    }

    // Top edge
    row = image[image.height - 1];
    for (ssize_t column_nr = 0; column_nr != image.width; ++column_nr) {
        auto &pixel = row[column_nr];
        if (static_cast<float>(pixel) > 0.0) {
            pixel.repair();
        }
    }

    // Left and right edge
    for (ssize_t row_nr = 0; row_nr != image.height; ++row_nr) {
        row = image[row_nr];

        auto &left_pixel = row[0];
        if (static_cast<float>(left_pixel) > 0.0) {
            left_pixel.repair();
        }

        auto &right_pixel = row[image.width - 1];
        if (static_cast<float>(right_pixel) > 0.0) {
            right_pixel.repair();
        }
    }
}

static void bad_pixels_horizontally(PixelMap<SDF8> &image) noexcept
{
    for (ssize_t row_nr = 0; row_nr != image.height; ++row_nr) {
        auto row = image[row_nr];
        // The left edge of the signed distance field should be outside of the glyph -float_max
        auto prev_pixel_value = SDF8(-std::numeric_limits<float>::max());
        for (ssize_t column_nr = 0; column_nr != image.width; ++column_nr) {
            auto &pixel = row[column_nr];
            ttlet pixel_value = static_cast<float>(pixel);

            ttlet normal_delta = std::abs(prev_pixel_value - pixel_value);
            ttlet flipped_delta = std::abs(prev_pixel_value - -pixel_value);

            if ((flipped_delta + 3.0) < normal_delta) {
                pixel = -pixel_value;
                prev_pixel_value = -pixel_value;
            } else {
                prev_pixel_value = pixel_value;
            }
        }
    }
}

[[nodiscard]] std::vector<std::pair<int,int>> bad_pixels_homogenious(PixelMap<SDF8> const &image) noexcept
{
    constexpr float threshold = 0.075f;

    auto r = std::vector<std::pair<int,int>>{};

    auto row = image.at(0);
    auto next_row = image.at(1);
    for (int row_nr = 1; row_nr != (image.height - 1); ++row_nr) {
        auto prev_row = row;
        row = next_row;
        next_row = image.at(row_nr + 1);

        for (int column_nr = 1; column_nr != (image.width - 1); ++column_nr) {
            ttlet &pixel = row[column_nr];

            auto area = std::array{
                static_cast<float>(prev_row[column_nr - 1]),
                static_cast<float>(prev_row[column_nr]),
                static_cast<float>(prev_row[column_nr + 1]),
                static_cast<float>(row[column_nr - 1]),
                static_cast<float>(pixel),
                static_cast<float>(row[column_nr + 1]),
                static_cast<float>(next_row[column_nr - 1]),
                static_cast<float>(next_row[column_nr]),
                static_cast<float>(next_row[column_nr + 1])
            };

            ttlet normal_mean = mean(area.cbegin(), area.cend());
            ttlet normal_stddev = stddev(area.cbegin(), area.cend(), normal_mean);

            static_assert(std::ssize(area) % 2 == 1);
            area[std::ssize(area) / 2] = -area[std::ssize(area) / 2];

            ttlet flipped_mean = mean(area.cbegin(), area.cend());
            ttlet flipped_stddev = stddev(area.cbegin(), area.cend(), flipped_mean);

            if ((flipped_stddev + threshold) < normal_stddev) {
                // Flipped pixels is more homogeneous.
                r.emplace_back(column_nr, row_nr);
            }
        }
    }
    return r;
}


void fill(PixelMap<SDF8> &image, std::vector<BezierCurve> const &curves) noexcept
{
    for (int row_nr = 0; row_nr != image.height; ++row_nr) {
        auto row = image.at(row_nr);
        auto y = static_cast<float>(row_nr);
        for (int column_nr = 0; column_nr != image.width; ++column_nr) {
            auto x = static_cast<float>(column_nr);
            row[column_nr] = generate_SDF8_pixel(vec::point(x, y), curves);
        }
    }

    bad_pixels_horizontally(image);
    bad_pixels_edges(image);

    std::vector<std::pair<int,int>> bad_pixel_list;
    for (int i = 0; i < 10; i++) {
        bad_pixel_list = bad_pixels_homogenious(image);
        if (std::ssize(bad_pixel_list) == 0) {
            break;
        }
    
        for (ttlet &[x, y]: bad_pixel_list) {
            image[y][x].repair();
        }
    }
}

}
