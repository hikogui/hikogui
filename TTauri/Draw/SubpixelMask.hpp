// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"
#include "QBezier.hpp"

namespace TTauri::Draw {

struct SubpixelMask : PixelMap<uint8_t> {
    enum class Orientation {
        RedLeft,
        RedRight,
        Unknown
    };

    SubpixelMask(size_t width, size_t height) : PixelMap(width, height) {}

    /*! Average R,G,B pixels.
     */
    void average() {
        assert(width % 3 == 0);

        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            auto row = (*this)[rowNr];
            for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.size()); columnNr += 3) {
                auto& r = row[columnNr];
                auto& g = row[columnNr + 1];
                auto& b = row[columnNr + 2];
                r = g = b = static_cast<uint8_t>((static_cast<uint16_t>(r) + static_cast<uint16_t>(g) + static_cast<uint16_t>(b)) / 3);
            }
        }
    }

    /*! Swap R and B values of each pixel.
     */
    void flip() {
        assert(width % 3 == 0);

        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            auto row = (*this)[rowNr];
            for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.size()); columnNr += 3) {
                std::swap(row[columnNr], row[columnNr + 2]);
            }
        }
    }

    void filter(Orientation subpixelOrientation) {
        switch (subpixelOrientation) {
        case Orientation::RedLeft:
            return;
        case Orientation::RedRight:
            return flip();
        case Orientation::Unknown:
            return average();
        }
    }

    /*! Render a single row of pixels.
     * Each row needs to be rendered 5 times as slightly different heights, performing super sampling.
     * Fully covered sub-pixels will have the value 0xff;
     */
    void renderRow(size_t rowY, std::vector<QBezier> const& curves) {
        // 5 times super sampling.
        gsl::span<uint8_t> row = at(rowY);
        for (float y = rowY + 0.1f; y < (rowY + 1); y += 0.2f) {
            renderSubRow(row, y, curves);
        }
    }

    void render(std::vector<QBezier> curves) {
        for (auto &curve: curves) {
            curve.scale({3.0, 1.0});
        }

        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            renderRow(rowNr, curves);
        }
    }

    static void renderFullPixels(gsl::span<uint8_t> row, size_t start, size_t size);


    static void renderPartialPixels(gsl::span<uint8_t> row, size_t i, float const startX, float const endX);

    /*! Render pixels in a row between two x values.
     * Fully covered sub-pixel will have the value 51.
     */
    static void renderRowSpan(gsl::span<uint8_t> row, float startX, float endX);

    /*! SuperSample a row of pixels once.
     * Fully covered sub-pixel will have the value 51
     */
    static void renderSubRow(gsl::span<uint8_t> row, float rowY, std::vector<QBezier> const& curves);

};

void composit(PixelMap<uint64_t>& under, uint64_t overColor, SubpixelMask const& mask);

/*! Composit colors from the color table based on the mask onto destination.
 * Mask should be subpixelFiltered before use.
 */
void composit(PixelMap<uint64_t>& under, Color_sRGBLinear overColor, SubpixelMask const& mask);


}