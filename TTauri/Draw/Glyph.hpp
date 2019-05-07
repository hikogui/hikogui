// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "QBezier.hpp"
#include "PixelMap.hpp"
#include <glm/glm.hpp>

namespace TTauri::Draw {

struct Glyph {
    constexpr float maximumSmoothAngle = 5.0f;
    constexpr float sinMaximumSmoothAngle = sinf(maximumSmoothAngle / 180.0f * pi);

    /*! List of quadratic bezier segments representing a glyph
     * This list includes all the outlines of a glyph.
     * The signed distance field rendering algorithm doesn't care about individual outlines.
     */
    std::vector<QBezier> segments;

    /*! A list of offsets into segements which start each contour.
     * Plus an extra entry for the end of the last contour.
     */
    std::vector<size_t> contours;

    /*! Check if the corner between two segments is sharp.
     */
    bool isSharp(QBezier const &segment0, QBezier const &segment1) {
        auto const D0 = segment0.directionAt(1.0f);
        auto const D1 = segment1.directionAt(0.0f);

        return absf(viktorCross(D0, D1) < sinMaximumSmoothAngle || glm::dot(D0, D1) > 0.0f;
    }

    /*! Give a color for each edge of a contour.
     */
    void assignMSDFColorsPerContour(size_t contourIndex) {
        for (size_t contourIndex = 0; contourIndex < countours.size() - 1; contourIndex++) {
            auto const beginIndex = countours.at(contourIndex);
            auto const endIndex = countours.at(contourIndex + 1);

            // First edge of a contour is coloured special, so that when we loop at the end
            // we never have the same colour. If the contour is a single edge then mark it white.
            uint8_t color = (endIndex - beginIndex) > 1 ? 0b101 : 0b111;
            auto &nextSegment = segment.at(i);
            for (size_t i = beginIndex; i < endIndex; i++) {
                auto &currentSegment = nextSegment;
                currentSegment.color = color;

                nextSegment = segment.at((i + 1) < endIndex ? i + 1 : 0);

                if (isShart(currentSegment, nextSegment)) {
                    color = (color == 0b110) ? 0b011 : 0b110;
                }
            }
        }
    }

    void transform(const glm::mat3x3 &M) {
        for (auto &segment: segments) {
            segment.transform(M);
        }
    }

    std::tuple<float, float, float> renderPixel(glm::vec2 const P) const {
        QBezier::ClosestPoint closestRed;
        QBezier::ClosestPoint closestGreen;
        QBezier::ClosestPoint closestBlue;

        for (auto &segment: segments) {
            auto const point = segment.closestPoint(P);
            if ((segment.color & 0b001) && point < closestRed) {
                closestRed = point;
            }
            if ((segment.color & 0b010) && point < closestGreen) {
                closestGreen = point;
            }
            if ((segment.color & 0b100) && point < closestBlue) {
                closestBlue = point;
            }
        }

        return {
            std::copysign(closestRed.pseudoDistance, closestRed.inside),
            std::copysign(closestGreen.pseudoDistance, closestGreen.inside),
            std::copysign(closestBlue.pseudoDistance, closestBlue.inside),
        }
    }

    uint32_t renderPixel(glm::vec2 const P, float distanceRange) const {
        auto const [red, green, blue] = renderPixel(P);

        return (
            static_cast<uint8_t>((std::clamp(red / distanceRange, -0.5f, 0.5f) + 0.5f) * 255.0f) |
            (static_cast<uint8_t>((std::clamp(green / distanceRange, -0.5f, 0.5f) + 0.5f) * 255.0f) << 8) |
            (static_cast<uint8_t>((std::clamp(blue / distanceRange, -0.5f, 0.5f) + 0.5f) * 255.0f) << 16)
        );
    }

    void renderMSDF(PixelMap<uint32_t> &map, float maxDistance) const {
        auto const distanceRange = 2.0f * maxDistance;

        for (size_t y = 0; y < map.height; y++) {
            auto row = map.a(y);
            for (size_t x = 0; x < map.width; x++) {
                auto const P = glm::vec2(x, y);
                row[x] = renderPixel(P, distanceRange);
            }
        }
    }
};


}