// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"

namespace TTauri::Draw {

void add1PixelTransparentBorder(PixelMap<uint32_t> &pixelMap)
{
    uint8_t const u8invisibleMask[4] = { 0xff, 0xff, 0xff, 0 };
    uint32_t u32invisibleMask;
    std::memcpy(&u32invisibleMask, u8invisibleMask, sizeof(u32invisibleMask));

    let topBorder = pixelMap.at(0);
    let topRow = pixelMap.at(1);
    let bottomRow = pixelMap.at(pixelMap.height - 2);
    let bottomBorder = pixelMap.at(pixelMap.height - 1);
    for (size_t x = 1; x < pixelMap.width - 1; x++) {
        topBorder[x] = topRow[x] & u32invisibleMask;
        bottomBorder[x] = bottomRow[x] & u32invisibleMask;
    }

    let rightBorderY = pixelMap.width - 1;
    let rightY = pixelMap.width - 2;
    for (size_t y = 1; y < pixelMap.height - 1; y++) {
        let row = pixelMap[y];
        row[0] = row[1] & u32invisibleMask;
        row[rightBorderY] = row[rightY] & u32invisibleMask;
    }

    pixelMap[0][0] = pixelMap[1][1] & u32invisibleMask;
    pixelMap[0][pixelMap.width - 1] = pixelMap[1][pixelMap.width - 2] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][0] = pixelMap[pixelMap.height - 2][1] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][pixelMap.width - 1] = pixelMap[pixelMap.height - 2][pixelMap.width - 2] & u32invisibleMask;
}

/*! Render a single pixel between two x values.
 * Fully covered sub-pixel will have the value 200.
 */
static void renderSubpixel(uint32_t &pixel, float beginX, float endX) {
    let leftLength = std::clamp(endX, 0.0f, 1.0f/3.0f) - std::clamp(beginX, 0.0f, 1.0f/3.0f);
    let midLength = std::clamp(endX, 1.0f/3.0f, 2.0f/3.0f) - std::clamp(beginX, 1.0f/3.0f, 2.0f/3.0f);
    let rightLength = std::clamp(endX, 2.0f/3.0f, 1.0f) - std::clamp(beginX, 2.0f/3.0f, 1.0f);

    // The length are between 0.0 and 0.3.. so multiply the bright value by 3 to compensate.
    pixel +=
        (static_cast<uint32_t>(leftLength * 600) << 20) |
        (static_cast<uint32_t>(midLength * 600) << 10) |
        static_cast<uint32_t>(rightLength * 600)
}

/*! Render pixels in a row between two x values.
 * Fully covered sub-pixel will have the value 200.
 */
static void renderSpan(gsl::span<uint32_t> row, float startX, float endX) {
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
        renderSubpixel(row[startColumn], startX - startColumn, endX - startColumn);

    } else {
        renderSubpixel(row[startColumn], startX - startColumn, 1.0f);

        for (int64_t i = startColumn + 1; i < (endColumn - 1); i++) {
            row[i] += ((200 << 20) | (200 << 10) | 200);
        }

        renderSubpixel(row[endColumn - 1], 0.0f, endX - (endColumn - 1)); 
    }
}

/*! SuperSample a row of pixels once.
 * Fully covered sub-pixel will have the value 0x33
 */
static void renderSuperSampleRow(gsl::span<uint32_t> row, float rowY, std::vector<QBezier> const& curves) {
    auto results = solveCurvesXByY(curves, rowY);
    if (results.size() == 0) {
        return;
    }

    std::sort(results.begin(), results.end());

    // For each span of x values.
    for (size_t i = 0; i < (results.size() / 2); i++) {
        let startX = results[i];
        let endX = results[i+1];
        renderSpan(row, startX, endX);
    }

    // the rest of the row is inside a curve.
    if (results.size() % 2 == 1) {
        let startX = results.back();
        let endX = static_cast<float>(row.size());
        renderSpan(row, startX, endX);
    }
}

/*! Render a single row of pixels.
 * Each row needs to be rendered 5 times as slightly different heights, performing super sampling.
 * Fully covered sub-pixels will have the value 0xff;
 */
static void renderRow(gsl::span<uint32_t> row, int64_t rowY, std::vector<QBezier> const &curves)
{
    let rowCurves = filterCurvesByY(curves, static_cast<float>(rowY), static_cast<float>(rowY + 1));

    // 5 times super sampling.
    for (float y = rowY + 0.1f; y < (rowY + 1); y += 0.2f) {
        renderSuperSampleRow(row, y, rowCurves);
    }
}

static void renderSubpixelMask(PixelMap<uint32_t> &mask, std::vector<QBezier> const &curves)
{
    let [startY, endY] = minmaxYOfCurves(curves);

    let startRow = std::max(boost::numeric_cast<int64_t>(floorf(startY)), 0LL);
    let endRow = std::min(boost::numeric_cast<int64_t>(ceilf(endY)), boost::numeric_cast<int64_t>(mask.height));

    for (auto rowNr = startRow; rowNr < endRow; rowNr++) {
        auto row = mask.at(rowNr);
        renderRow(row, rowNr, curves);
    }
}

void renderSubpixelMask(PixelMap<uint32_t>& mask, Path const& path)
{
    return renderSubpixelMask(mask, path.getQBeziers());
}


/*! Set subpixel value, or don't do anything when writing beyond row.
 * clamp value to 1000.
 */
static void setSubpixel(gsl::span<uint32_t> row, size_t columnNr, uint16_t value)
{
    if (value > 1000) {
        value = 1000;
    }

    if ((columnNr / 3) < static_cast<size_t>(row.size())) {
        auto &pixel = row[columnNr / 3];
        switch (columnNr % 3) {
        case 0:
            pixel = (pixel & 0x000fffff) | (value << 20);
            return;
        case 1:
            pixel = (pixel & 0xfff003ff) | (value << 10);
            return;
        case 2:
            pixel = (pixel & 0xfffffc00) | value;
            return;
        }
    }
}

/*! Get subpixel value, or 0 when reading beyond row.
 */
static uint16_t getSubpixel(gsl::span<uint32_t> row, size_t columnNr)
{
    if ((columnNr / 3) < static_cast<size_t>(row.size())) {
        let pixel = row[columnNr / 3];
        switch (columnNr % 3) {
        case 0: return (pixel >> 20) & 0x3ff;
        case 1: return (pixel >> 10) & 0x3ff;
        case 2: return pixel & 0x3ff;
        default: return 0;
        }

    } else {
        return 0;
    }
}

/*! subpixel filter for RGB or BGR LCD displays.
 * The algorithm finds triplers of subpixels that have
 * different values and averages them. The start of a triplet
 * may inside an actual pixel.
 */
static void subpixelFilteringLCD(PixelMap<uint32_t>& pixels)
{
    for (size_t rowNr = 0; rowNr < pixels.height; rowNr++) {
        auto row = pixels[rowNr];

        bool same = true;
        uint16_t prevValue = 0;
        uint16_t prevPrevValue = 0;
        for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.size() * 3);) {
            let value = getSubpixel(row, columnNr);

            if (value == prevValue && value == prevPrevValue) {
                // Nothing changes.
                columnNr++;
                prevPrevValue = prevValue;
                prevValue = value;

            } else {
                // Change three subpixels at once with the average.
                let nextValue = getSubpixel(row, columnNr + 1);
                let nextNextValue = getSubpixel(row, columnNr + 2);
                let average = (value + nextValue + nextNextValue) / 3;
                setSubpixel(row, columnNr, average);
                setSubpixel(row, columnNr + 1, average);
                setSubpixel(row, columnNr + 2, average);
                columnNr+= 3;
                prevPrevValue = average;
                prevValue = average;
            }
        }
    }
}

/*! subpixel filter for unknown subpixel orientation.
 * This function will simply everage out the RGB value of each pixel.
 */
static void subpixelFilteringUnknown(PixelMap<uint32_t>& pixels)
{
    for (size_t rowNr = 0; rowNr < pixels.height; rowNr++) {
        auto row = pixels[rowNr];
        for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.size()); columnNr++) {
            auto& pixel = row[columnNr];
            let oldPixel = pixel;
            uint32_t value = (((oldPixel >> 20) & 0x3ff) + ((oldPixel >> 10) & 0x3ff) + (oldPixel & 0x3ff)) / 3;
            pixel = (value << 20) | (value << 10) | value;
        }
    }
}

/*! Swap R and B values of each pixel.
 */
static void subpixelFlip(PixelMap<uint32_t>& pixels)
{
    for (size_t rowNr = 0; rowNr < pixels.height; rowNr++) {
        auto row = pixels[rowNr];
        for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.size()); columnNr++) {
            auto &pixel = row[columnNr];
            let oldPixel = pixel;
            pixel = ((oldPixel & 0x3dd) << 20) | ((oldPixel & 0x000ffc00 | ((oldPixel >> 20) & 0x3ff);
        }
    }
}

void subpixelFiltering(PixelMap<uint32_t> &pixels, SubpixelOrientation subpixelOrientation)
{
    switch (subpixelOrientation) {
    case SubpixelOrientation::RedLeft:
        subpixelFilteringLCD(pixels);
        return;
    case SubpixelOrientation::RedRight:
        subpixelFilteringLCD(pixels);
        subpixelFlip(pixels);
        return;
    case SubpixelOrientation::Unknown:
        subpixelFilteringUnknown(pixels);
        return;
    }
}

/*! Composit colors from the color table based on the mask onto destination.
 */
void subpixelComposit(PixelMap<uint32_t>& destination, PixelMap<uint32_t> const& source, PixelMap<uint32_t> const& mask, Color_XYZ color)
{
    assert(destination.height >= source.height);
    assert(destination.width >= source.width);
    assert(mask.height >= source.height);
    assert(mask.width >= source.width);

    let overColor = color_cast<Color_sRGBLinear>(color);

    for (size_t rowNr = 0; rowNr < source.height; rowNr++) {
        auto sourceRow = source[rowNr];
        auto maskRow = mask[rowNr];
        auto destinationRow = destination[rowNr];
        for (size_t columnNr = 0; columnNr < static_cast<size_t>(sourceRow.size()); columnNr++) {
            let sourcePixel = sourceRow[columnNr];
            let maskPixel = maskRow[columnNr];

            if (maskPixel > 0) {
                let underColor = Color_sRGB(sourcePixel).toLinear();
                let indexColor = maskPixel & 0xff;
                let subpixelMask = glm::vec3{
                    ((maskPixel >> 20) & 0x3ff) / 1000.0,
                    ((maskPixel >> 10) & 0x3ff) / 1000.0,
                    ((maskPixel & 0x3ff) / 1000.0
                };

                let destinationColor = underColor.composit(overColor, subpixelMask);

            } else {
                // Shortcut for fully transparent mask pixel.
                destinationRow[columnNr] = sourcePixel;
            }
        }
    }
}

/*! Composit colors from the color table based on the mask onto destination.
 * Mask should be subpixelFiltered before use.
 */
void subpixelComposit(PixelMap<uint32_t>& destination, PixelMap<uint32_t> const& mask, Color_XYZ color)
{
    return subpixelComposit(destination, destination, mask, color);
}

void render(PixelMap<uint32_t>& pixels, Path const& path, Color_XYZ color, SubpixelOrientation subpixelOrientation)
{
    auto mask = PixelMap<uint32_t>(pixels.width, pixels.height);
    mask.clear();
    renderSubpixelMask(mask, path);
    subpixelFiltering(mask, subpixelOrientation);

    subpixelComposit(pixels, mask, color);
}


}
