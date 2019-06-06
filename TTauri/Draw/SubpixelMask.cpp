// Copyright 2019 Pokitec
// All rights reserved.

#include "SubpixelMask.hpp"
#include "TTauri/all.hpp"
#include <algorithm>

namespace TTauri::Draw {



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