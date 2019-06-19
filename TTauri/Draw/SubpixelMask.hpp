// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"
#include "Bezier.hpp"

namespace TTauri {
struct wsRGBApm;
}

namespace TTauri::Draw {

struct SubpixelMask : PixelMap<uint8_t> {
    enum class Orientation {
        RedLeft,
        RedRight,
        Unknown
    };

    SubpixelMask(size_t width, size_t height) : PixelMap(width, height) {}

    /*! Average each RGB pixel.
     * This results in a standard anti-aliased image.
     */
    void averageRGB() {
        assert(width % 3 == 0);

        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            auto row = (*this)[rowNr];
            for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.width); columnNr += 3) {
                auto& r = row[columnNr];
                auto& g = row[columnNr + 1];
                auto& b = row[columnNr + 2];
                r = g = b = static_cast<uint8_t>((static_cast<uint16_t>(r) + static_cast<uint16_t>(g) + static_cast<uint16_t>(b)) / 3);
            }
        }
    }

    /*! Swap R and B values of each RGB pixel.
     */
    void flipRGB() {
        assert(width % 3 == 0);

        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            auto row = (*this)[rowNr];
            for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.width); columnNr += 3) {
                std::swap(row[columnNr], row[columnNr + 2]);
            }
        }
    }

    /*! Reduce colour fringe for subpixels.
     */
    void smoothRGB() {
        horizontalFilter<5>(*this, [](auto values) {
            return static_cast<uint8_t>((
                (values & 0xff) +
                ((values >> 8) & 0xff) * 2 +
                ((values >> 16) & 0xff) * 3 +
                ((values >> 24) & 0xff) * 2 +
                ((values >> 32) & 0xff)
            ) / 9);
        });
    }

    void filter(Orientation subpixelOrientation) {
        switch (subpixelOrientation) {
        case Orientation::RedLeft:
            smoothRGB();
            return;

        case Orientation::RedRight:
            smoothRGB();
            flipRGB();
            return;

        case Orientation::Unknown:
            averageRGB();
            return;
        }
    }

    void fill(std::vector<Bezier> curves) {
        for (auto& curve : curves) {
            curve.scale({ 3.0, 1.0 });
        }

        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            auto row = at(rowNr);
            fillRow(row, rowNr, curves);
        }
    }
};

/*! Composit colors from the color table based on the mask onto destination.
 * Mask should be subpixelFiltered before use.
 */
void composit(PixelMap<wsRGBApm>& under, wsRGBApm overColor, SubpixelMask const& mask);


}
