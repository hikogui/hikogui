// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "TTauri/utils.hpp"
#include "TTauri/geometry.hpp"

namespace TTauri::Draw {

struct BezierPoint {
    glm::vec2 p;
    bool onCurve;

    BezierPoint(glm::vec2 p, bool onCurve) : p(p), onCurve(onCurve) {}
    BezierPoint(float x, float y, bool onCurve) : BezierPoint({x, y}, onCurve) {}

    bool operator==(BezierPoint const &other) const {
        return (p == other.p) && (onCurve == other.onCurve);
    }

    BezierPoint transform(glm::vec2 position, float scale=1.0f, float rotate=0.0f) const {
        let newP = glm::rotate(p * scale, rotate) + position;
        return { newP, onCurve };
    }

    static BezierPoint midpoint(BezierPoint a, BezierPoint b)
    {
        assert(a.onCurve == b.onCurve);
        return { TTauri::midpoint(a.p, b.p), !a.onCurve };
    }

    /*!
     *
     * \return A list of points, starting with an onCurve point. Each onCurve point is followed by an offCurve point.
     */
    static std::vector<BezierPoint> normalizePoints(std::vector<BezierPoint> const& points)
    {
        assert(points.size() >= 2);

        std::vector<BezierPoint> normalizedPoints;

        auto previousPoint = points.back();

        for (let point : points) {
            // Make sure that every onCurve point is acompanied by a offCurve point.
            if (previousPoint.onCurve == point.onCurve) {
                normalizedPoints.push_back(BezierPoint::midpoint(previousPoint, point));
            }
            normalizedPoints.push_back(point);

            previousPoint = point;
        }

        if (!normalizedPoints.front().onCurve) {
            // Make sure the first point lays on the curve.
            std::rotate(normalizedPoints.begin(), normalizedPoints.begin() + 1, normalizedPoints.end());
        }

        assert(normalizedPoints.size() % 2 == 0);
        return normalizedPoints;
    }
};


}