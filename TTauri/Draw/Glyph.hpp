// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "QBezier.hpp"
#include "PixelMap.hpp"
#include "TTauri/geometry.hpp"

#include <glm/glm.hpp>

namespace TTauri::Draw {


/*!
 *
 * \return A list of points, starting with an onCurve point. Each onCurve point is followed by an offCurve point.
 */
static std::vector<glm::vec2> normalizePoints(std::vector<std::pair<glm::vec2, bool>> const& points)
{
    assert(points.size() >= 2);

    std::vector<glm::vec2> r;

    let [firstCoord, firstOnCurve] = points.front();

    glm::vec2 previousCoord;
    bool previousOnCurve;
    std::tie(previousCoord, previousOnCurve) = points.back();

    bool swapLast = !firstOnCurve && previousOnCurve;
    if (swapLast) {
        r.emplace_back(previousCoord);
    }

    for (let [coord, onCurve] : points) {
        if (previousOnCurve == onCurve) {
            r.emplace_back(midpoint(previousCoord, coord));
        }

        r.emplace_back(coord);
        previousCoord = coord;
        previousOnCurve = onCurve;
    }

    bool lastOnCurve = previousOnCurve;
    if (swapLast) {
        // Last point was already used as the first point.
        r.pop_back();
        lastOnCurve = points.at(points.size() - 2).second;
    }

    if (lastOnCurve) {
        r.emplace_back(midpoint(r.back(), r.front()));
    }

    return r;
}

static std::vector<QBezier> getContour(std::vector<std::pair<glm::vec2, bool>> const& points) {
    std::vector<QBezier> contour;

    let normalizedPoints = normalizePoints(points);
    for (auto i = normalizedPoints.begin(); i != normalizedPoints.end();) {
        let onCurvePoint = *(i++);
        let offCurvePoint = *(i++);

        if (contour.size()) {
            contour.back().P2 = onCurvePoint;
        }

        contour.emplace_back(onCurvePoint, offCurvePoint, glm::vec2{});
    }
    contour.back().P2 = contour.front().P0;

    return contour;
}




struct Glyph {
    /*! List of quadratic bezier segments representing a glyph
     * This list includes all the outlines of a glyph.
     * The signed distance field rendering algorithm doesn't care about individual outlines.
     */
    std::vector<QBezier> segments;

    void transform(const glm::mat3x3 &M) {
        for (auto &segment: segments) {
            segment.transform(M);
        }
    }

    /*!
     * \param points <coordinate, onCurve>
     */
    void addContour(std::vector<std::pair<glm::vec2, bool>> const &points) {
        let contour = getContour(points);

        for (let segment: contour) {
            segments.emplace_back(segment);
        }
    }

};


}