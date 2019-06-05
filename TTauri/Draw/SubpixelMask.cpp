// Copyright 2019 Pokitec
// All rights reserved.

#include "SubpixelMask.hpp"
#include "TTauri/all.hpp"
#include <algorithm>

namespace TTauri::Draw {

void SubpixelMask::renderPartialPixels(gsl::span<uint8_t> row, size_t i, float const startX, float const endX)
{
    let pixelCoverage =
        std::clamp(endX, i + 0.0f, i + 1.0f) -
        std::clamp(startX, i + 0.0f, i + 1.0f);

    auto &pixel = row[i];
    pixel = static_cast<uint8_t>(std::min(pixelCoverage * 51.0f + pixel, 255.0f));
}

void SubpixelMask::renderFullPixels(gsl::span<uint8_t> row, size_t start, size_t size)
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
        let alignedStart = TTauri::align<uint8_t *>(u8p, sizeof(uint64_t));
        while (u8p < alignedStart) {
            *(u8p++) += 0x33;
        }

        // add 51 for each pixel, 8 pixels at a time.
        auto u64p = reinterpret_cast<uint64_t *>(u8p);
        let u64end = TTauri::align_end<uint64_t *>(u8end, sizeof(uint64_t));
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

void SubpixelMask::renderRowSpan(gsl::span<uint8_t> row, float const startX, float const endX) {
    if (startX >= row.size() || endX < 0.0f) {
        return;
    }

    let startX_int = static_cast<int64_t>(startX);
    let endX_int = static_cast<int64_t>(endX + 1.0f);
    let startColumn = std::max(startX_int, static_cast<int64_t>(0));
    let endColumn = std::min(endX_int, row.size());
    let nrColumns = endColumn - startColumn;

    if (nrColumns == 1) {
        renderPartialPixels(row, startColumn, startX, endX);

    } else {
        renderPartialPixels(row, startColumn, startX, endX);
        renderFullPixels(row, startColumn + 1, nrColumns - 2);
        renderPartialPixels(row, endColumn - 1, startX, endX);
    }
}

void SubpixelMask::renderSubRow(gsl::span<uint8_t> row, float rowY, std::vector<QBezier> const& curves) {
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

void composit(PixelMap<uint64_t> &under, uint64_t overColor, SubpixelMask const& mask)
{
    assert(mask.height >= under.height);
    assert((mask.width * 3) >= under.width);

    for (size_t rowNr = 0; rowNr < under.height; rowNr++) {
        auto maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (size_t maskColumnNr = 0, columnNr = 0; columnNr < static_cast<size_t>(underRow.size()); columnNr++, maskColumnNr += 3) {
            let maskRed = maskRow[maskColumnNr];
            let maskGreen = maskRow[maskColumnNr + 1];
            let maskBlue = maskRow[maskColumnNr + 2];

            if (maskRed > 0 || maskGreen > 0 || maskBlue > 0) {
                auto& underPixel = underRow[columnNr];
                let underColor = underPixel;

                // 16 bit (65535).
                constexpr uint64_t _overVmax = 0xffff;
                let _overV = glm::u64vec4{
                    (overColor >> 48) & _overVmax,
                    (overColor >> 32) & _overVmax,
                    (overColor >> 16) & _overVmax,
                    overColor & _overVmax
                };

                // 8 bit (255).
                constexpr uint64_t _maskVmax = 0xff;
                let _maskV = glm::u64vec4{
                    maskRed, maskGreen, maskBlue,
                    (static_cast<uint16_t>(maskRed) + static_cast<uint16_t>(maskGreen) + static_cast<uint16_t>(maskBlue)) / 3
                };

                // The final mask for each component is the subpixel multiplied by the alpha of over.
                // 8 bit * 16 bit = 24 bit (16711425).
                constexpr uint64_t maskVmax = _maskVmax * _overVmax;
                let maskV = _maskV * _overV.a;

                // 24 bit.
                constexpr uint64_t oneMinusMaskVmax = maskVmax;
                let oneMinusMaskV = glm::u64vec4{ maskVmax, maskVmax, maskVmax, maskVmax } - maskV;

                // Final over-color is to multiply each color component with the final mask
                // 16 bit * 24 bit = 40 bit
                constexpr uint64_t overVmax = _overVmax * maskVmax;
                let overV = _overV * maskV;

                // 16 bit.
                constexpr uint64_t underVmax = 0xffff;
                let underV = glm::u64vec4{
                    (underColor >> 48) & underVmax,
                    (underColor >> 32) & underVmax,
                    (underColor >> 16) & underVmax,
                    underColor & underVmax
                };

                // 40bit + (16bit * 24bit) = 40bit + 40bit = 40bit.
                constexpr uint64_t resultVmax = std::max(overVmax, underVmax * oneMinusMaskVmax);
                let resultV = overV + underV * oneMinusMaskV;

                constexpr uint64_t resultVdivider = resultVmax / 0xffff;
                let u16resultV = resultV / resultVdivider;

                uint64_t result = (
                    (u16resultV.r << 48) |
                    (u16resultV.g << 32) |
                    (u16resultV.b << 16) |
                    u16resultV.a
                );

                underPixel = result;
            }
        }
    }
}

void composit(PixelMap<uint64_t> &under, Color_sRGBLinear overColor, SubpixelMask const &mask)
{
    return composit(under, overColor.toUInt64PreMultipliedAlpha(), mask);
}

}