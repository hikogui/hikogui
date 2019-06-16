// Copyright 2019 Pokitec
// All rights reserved.

#include "Bezier.hpp"
#include "BezierPoint.hpp"
#include "TTauri/utils.hpp"

namespace TTauri::Draw {

std::vector<Bezier> Bezier::getContour(std::vector<BezierPoint> const& _points)
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


static void renderPartialPixels(gsl::span<uint8_t> row, size_t i, float const startX, float const endX)
{
    let pixelCoverage =
        std::clamp(endX, i + 0.0f, i + 1.0f) -
        std::clamp(startX, i + 0.0f, i + 1.0f);

    auto & pixel = row[i];
    pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
}

static void renderFullPixels(gsl::span<uint8_t> row, size_t start, size_t size)
{
    if (size < 16) {
        let end = start + size;
        for (size_t i = start; i < end; i++) {
            row[i] += 0x33;
        }
    }
    else {
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
static void renderRowSpan(gsl::span<uint8_t> row, float const startX, float const endX) {
    if (startX >= row.size() || endX < 0.0f) {
        return;
    }

    let startX_int = static_cast<int64_t>(startX);
    let endXplusOne = endX + 1.0f;
    let endX_int = static_cast<int64_t>(endXplusOne);
    let startColumn = std::max(startX_int, static_cast<int64_t>(0));
    let endColumn = std::min(endX_int, row.size());
    let nrColumns = endColumn - startColumn;

    if (nrColumns == 1) {
        renderPartialPixels(row, startColumn, startX, endX);

    }
    else {
        renderPartialPixels(row, startColumn, startX, endX);
        renderFullPixels(row, startColumn + 1, nrColumns - 2);
        renderPartialPixels(row, endColumn - 1, startX, endX);
    }
}

static void renderSubRow(gsl::span<uint8_t> row, float rowY, std::vector<Bezier> const& curves) {
    auto results = solveCurvesXByY(curves, rowY);
    if (results.size() == 0) {
        return;
    }

    // Each closed path will result in intersection with a line in pairs.
    assert(results.size() % 2 == 0);

    std::sort(results.begin(), results.end());

    // For each span of x values.
    for (size_t i = 0; i < results.size(); i += 2) {
        let startX = results[i];
        let endX = results[i + 1];
        renderRowSpan(row, startX, endX);
    }
}

void renderRow(gsl::span<uint8_t> row, size_t rowY, std::vector<Bezier> const& curves) {
    // 5 times super sampling.
    for (float y = rowY + 0.1f; y < (rowY + 1); y += 0.2f) {
        renderSubRow(row, y, curves);
    }
}



}