// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "BezierPoint.hpp"
#include "TTauri/utils.hpp"
#include "TTauri/math.hpp"
#include "TTauri/geometry.hpp"

#include <glm/glm.hpp>
#include <tuple>
#include <limits>
#include <algorithm>

namespace TTauri::Draw {

/*! Quadratic Bezier Curve
*/
struct QBezier {
    glm::vec2 P0; //!< First point
    glm::vec2 P1; //!< Control point
    glm::vec2 P2; //!< Last point

    QBezier(glm::vec2 P0, glm::vec2 P1, glm::vec2 P2) : P0(P0), P1(P1), P2(P2) {}

    void transform(glm::mat3x3 M) {
        P0 = (glm::vec3(P0, 1.0) * M).xy();
        P1 = (glm::vec3(P1, 1.0) * M).xy();
        P2 = (glm::vec3(P2, 1.0) * M).xy();
    }

    std::pair<float,float> minmaxY() const {
        return std::minmax({P0.y, P1.y, P2.y});
    }

    static std::vector<QBezier> getContour(std::vector<BezierPoint> const& points) {
        std::vector<QBezier> contour;

        let normalizedPoints = BezierPoint::normalizePoints(points);
        for (auto i = normalizedPoints.begin(); i != normalizedPoints.end();) {
            let onCurvePoint = *(i++);
            assert(onCurvePoint.onCurve);
            let offCurvePoint = *(i++);
            assert(!offCurvePoint.onCurve);

            if (contour.size()) {
                contour.back().P2 = onCurvePoint.p;
            }

            contour.emplace_back(onCurvePoint.p, offCurvePoint.p, glm::vec2{});
        }
        contour.back().P2 = contour.front().P0;

        return contour;
    }

    results2 solveTByY(float y) const {
        let a = P0.y + 2.0f*P1.y + P2.y;
        let b = -2.0f*P0.y + 2.0f*P1.y;
        let c = P0.y;
        return solveQuadratic(a, b, c - y);
    }

    results2 solveXByY(float y) const {
        results2 r;

        for (let t: solveTByY(y)) {
            if (t >= 0.0f && t <= 1.0f) {
                let a = P0.x + 2.0f * P1.x + P2.x;
                let b = -2.0f * P0.x + 2.0f * P1.x;
                let c = P0.x;
                r.add(a*t*t + b*t + c);
            }
        }

        return r;
    }

};

inline std::pair<float, float> minmaxYOfCurves(std::vector<QBezier> const& v) {
    float minimum = std::numeric_limits<float>::infinity();
    float maximum = -std::numeric_limits<float>::infinity();

    for (let& curve : v) {
        let[curveMinimumY, curveMaximumY] = curve.minmaxY();
        minimum = std::min(minimum, curveMinimumY);
        maximum = std::min(maximum, curveMaximumY);
    }
    return { minimum, maximum };
}

inline std::vector<QBezier> filterCurvesByY(std::vector<QBezier> const &v, float minimumY, float maximumY) {
    std::vector<QBezier> r;
    r.reserve(v.size());

    for (let &curve: v) {
        let [curveMinimumY, curveMaximumY] = curve.minmaxY();
        if (!(curveMaximumY < minimumY || curveMinimumY > maximumY)) {
            r.push_back(curve);
        }
    }

    return r;
}

inline std::vector<float> solveCurvesXByY(std::vector<QBezier> const &v, float y) {
    std::vector<float> r;
    r.reserve(v.size());

    for (let &curve: v) {
        for (let x: curve.solveXByY(y)) {
            r.push_back(x);
        }
    }
    return r;
}


}