// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"

namespace TTauri::Draw {

void add1PixelTransparentBorder(PixelMap<uint32_t> pixelMap)
{
    uint8_t const u8invisibleMask[4] = { 0xff, 0xff, 0xff, 0 };
    uint32_t u32invisibleMask;
    std::memcpy(&u32invisibleMask, u8invisibleMask, sizeof(u32invisibleMask));

    auto const topBorder = pixelMap.at(0);
    auto const topRow = pixelMap.at(1);
    auto const bottomRow = pixelMap.at(pixelMap.height - 2);
    auto const bottomBorder = pixelMap.at(pixelMap.height - 1);
    for (size_t x = 1; x < pixelMap.width - 1; x++) {
        topBorder[x] = topRow[x] & u32invisibleMask;
        bottomBorder[x] = bottomRow[x] & u32invisibleMask;
    }

    auto const rightBorderY = pixelMap.width - 1;
    auto const rightY = pixelMap.width - 2;
    for (size_t y = 1; y < pixelMap.height - 1; y++) {
        auto const row = pixelMap[y];
        row[0] = row[1] & u32invisibleMask;
        row[rightBorderY] = row[rightY] & u32invisibleMask;
    }

    pixelMap[0][0] = pixelMap[1][1] & u32invisibleMask;
    pixelMap[0][pixelMap.width - 1] = pixelMap[1][pixelMap.width - 2] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][0] = pixelMap[pixelMap.height - 2][1] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][pixelMap.width - 1] = pixelMap[pixelMap.height - 2][pixelMap.width - 2] & u32invisibleMask;
}

static void renderSubpixel(uint32_t &pixel, float beginX, float endX, uint8_t colorIndex) {
    pixel &= 0xffffff00;

    let leftLength = std::clamp(endX, 0.0f, 1.0f/3.0f) - std::clamp(beginX, 0.0f, 1.0f/3.0f);
    let midLength = std::clamp(endX, 1.0f/3.0f, 2.0f/3.0f) - std::clamp(beginX, 1.0f/3.0f, 2.0f/3.0f);
    let rightLength = std::clamp(endX, 2.0f/3.0f, 1.0f) - std::clamp(beginX, 2.0f/3.0f, 1.0f);

    let brighten =
        (static_cast<uint32_t>(leftLength * 0x30) << 24) |
        (static_cast<uint32_t>(midLength * 0x30) << 16) |
        (static_cast<uint32_t>(rightLength * 0x30) << 8) |
        colorIndex;
    pixel += brighten;
}

static void renderSpan(gsl::span<uint32_t> row, float startX, float endX, uint8_t colorIndex) {
    if (startX >= row.size() || endX < 0.0f) {
        return;
    }

    // Get the start and end pixel that needs to be lightened.
    startX = std::max(startX, 0.0f);
    endX = std::min(endX, static_cast<float>(row.size()));
    let lengthX = endX - startX;

    int64_t startColumn = static_cast<int64_t>(floorf(startX));
    int64_t endColumn = static_cast<int64_t>(ceilf(endX));
    int64_t nrColumns = endColumn - startColumn;

    if (nrColumns == 0) {
        return;

    } else if (nrColumns == 1) {
        renderSubpixel(row[startColumn], startX - startColumn, endX - startColumn, colorIndex);

    } else {
        renderSubpixel(row[startColumn], startX - startColumn, 1.0f, colorIndex);

        let brighten = 0x101010 + colorIndex;
        for (int64_t i = startColumn + 1; i < (endColumn - 1); i++) {
            auto pixel = row[i] & 0xffffff00;
            row[i] = pixel + brighten;
        }

        renderSubpixel(row[endColumn - 1], 0.0f, endX - (endColumn - 1), colorIndex); 
    }
}

static void renderSuperSampleRow(gsl::span<uint32_t> row, float rowY, std::vector<QBezier> const& curves, uint8_t colorIndex) {
    auto results = solveCurvesXByY(curves, rowY);
    if (results.size() == 0) {
        return;
    }

    std::sort(results.begin(), results.end());

    // For each span of x values.
    for (size_t i = 0; i < (results.size() / 2); i++) {
        let startX = results[i];
        let endX = results[i+1];
        renderSpan(row, startX, endX, colorIndex);
    }

    // the rest of the row is inside a curve.
    if (results.size() % 2 == 1) {
        let startX = results.back();
        let endX = static_cast<float>(row.size());
        renderSpan(row, startX, endX, colorIndex);
    }
}

static void renderRow(gsl::span<uint32_t> row, int64_t rowY, std::vector<QBezier> const &curves, uint8_t colorIndex)
{
    let rowCurves = filterCurvesByY(curves, static_cast<float>(rowY), static_cast<float>(rowY + 1));

    // 8 times super sampling.
    for (float y = rowY + 0.0625f; y < (rowY + 1); y += 0.125f) {
        renderSuperSampleRow(row, y, rowCurves, colorIndex);
    }
}

void render(PixelMap<uint32_t> pixels, std::vector<QBezier> const &curves, uint8_t colorIndex)
{
    let [startY, endY] = minmaxYOfCurves(curves);

    let startRow = std::max(boost::numeric_cast<int64_t>(floorf(startY)), 0LL);
    let endRow = std::min(boost::numeric_cast<int64_t>(ceilf(endY)), boost::numeric_cast<int64_t>(pixels.height));

    for (auto rowNr = startRow; rowNr < endRow; rowNr++) {
        auto row = pixels.at(rowNr);
        renderRow(row, rowNr, curves, colorIndex);
    }
}


}