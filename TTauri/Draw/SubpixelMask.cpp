// Copyright 2019 Pokitec
// All rights reserved.

#include "SubpixelMask.hpp"
#include <algorithm>

namespace TTauri::Draw {

void SubpixelMask::renderRowSpan(gsl::span<uint8_t> row, float const startX, float const endX) {
    if (startX >= row.size() || endX < 0.0f) {
        return;
    }

    let startX_int = static_cast<int64_t>(floorf(startX));
    let endX_int = static_cast<int64_t>(ceilf(endX));
    let startColumn = std::max(startX_int, static_cast<int64_t>(0));
    let endColumn = std::min(endX_int, row.size());

    for (auto i = startColumn; i < endColumn; i++) {
        auto& pixel = row[i];
        let pixelCoverage = std::clamp(endX, i + 0.0f, i + 1.0f) - std::clamp(startX, i + 0.0f, i + 1.0f);
        
        pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
    }
}

void SubpixelMask::renderSubRow(gsl::span<uint8_t> row, float rowY, std::vector<QBezier> const& curves) {
    auto results = solveCurvesXByY(curves, rowY);
    if (results.size() == 0) {
        return;
    }

    std::sort(results.begin(), results.end());

    // For each span of x values.
    let shortSize = (results.size() / 2) * 2;
    for (size_t i = 0; i < shortSize; i += 2) {
        let startX = results[i];
        let endX = results[i + 1];
        renderRowSpan(row, startX, endX);
    }

    // the rest of the row is inside a curve.
    if (results.size() % 2 == 1) {
        let startX = results.back();
        let endX = static_cast<float>(row.size());
        renderRowSpan(row, startX, endX);
    }
}

void composit(PixelMap<uint32_t>& under, Color_sRGBLinear overColor, SubpixelMask const& mask)
{
    assert(mask.height >= under.height);
    assert((mask.width * 3) >= under.width);

    for (size_t rowNr = 0; rowNr < under.height; rowNr++) {
        auto maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (size_t maskColumnNr = 0, columnNr = 0; columnNr < static_cast<size_t>(underRow.size()); columnNr++, maskColumnNr += 3) {
            let maskRedPixel = maskRow[maskColumnNr];
            let maskGreenPixel = maskRow[maskColumnNr + 1];
            let maskBluePixel = maskRow[maskColumnNr + 2];

            if (maskRedPixel > 0 || maskGreenPixel > 0 || maskBluePixel > 0) {
                let maskSubpixelVector = glm::vec3{
                    maskRedPixel / 255.0f,
                    maskGreenPixel / 255.0f,
                    maskBluePixel / 255.0f
                };

                auto& underPixel = underRow[columnNr];
                let underColor = color_cast<Color_sRGBLinear>(Color_sRGB::readPixel(underPixel));

                let destinationColor = underColor.composit(overColor, maskSubpixelVector);

                underPixel = color_cast<Color_sRGB>(destinationColor).writePixel();
            }
        }
    }
}

}