// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/math.hpp"
#include "TTauri/geometry.hpp"
#include "TTauri/required.hpp"

#include <glm/glm.hpp>
#include <tuple>
#include <limits>
#include <algorithm>

namespace TTauri::Draw {

struct BezierPoint;

/*! Quadratic Bezier Curve
*/
struct Bezier {
    enum class Type { None, Linear, Quadratic, Cubic };

    Type type;
    glm::vec2 P1; //!< First point
    glm::vec2 C1; //!< Control point
    glm::vec2 C2; //!< Control point
    glm::vec2 P2; //!< Last point

    Bezier() : type(Type::None), P1(), C1(), C2(), P2() {}

    Bezier(glm::vec2 P1, glm::vec2 P2) : type(Type::Linear), P1(P1), C1(), C2(), P2(P2) {}

    Bezier(glm::vec2 P1, glm::vec2 C1, glm::vec2 P2) : type(Type::Quadratic), P1(P1), C1(C1), C2(), P2(P2) {}

    Bezier(glm::vec2 P1, glm::vec2 C1, glm::vec2 C2, glm::vec2 P2) : type(Type::Cubic), P1(P1), C1(C1), C2(C2), P2(P2) {}

    bool operator==(Bezier const &other) const {
        if (type != other.type) {
            return false;
        }
        switch (type) {
        case Type::None:
            return true;
        case Type::Linear:
            return (P1 == other.P1) && (P2 == other.P2);
        case Type::Quadratic:
            return (P1 == other.P1) && (C1 == other.C1) && (P2 == other.P2);
        case Type::Cubic:
            return (P1 == other.P1) && (C1 == other.C1) && (C2 == other.C2) && (P2 == other.P2);
        default:
            no_default;
        }
    }

    void transform(glm::mat3x3 M) {
        P1 = (glm::vec3(P1, 1.0) * M).xy();
        C1 = (glm::vec3(C1, 1.0) * M).xy();
        C2 = (glm::vec3(C2, 1.0) * M).xy();
        P2 = (glm::vec3(P2, 1.0) * M).xy();
    }

    void scale(glm::vec2 s) {
        P1 *= s;
        C1 *= s;
        C2 *= s;
        P2 *= s;
    }

    static std::vector<Bezier> getContour(std::vector<BezierPoint> const& points);

    results2 quadraticSolveTByY(float y) const {
        let a = P1.y - 2.0f*C1.y + P2.y;
        let b = 2.0f*(C1.y - P1.y);
        let c = P1.y;
        return solveQuadratic(a, b, c - y);
    }

    // x(t) = (-a + 3b- 3c + d)t³ + (3a - 6b + 3c)t² + (-3a + 3b)t + a
    // x(t) = (-P1 + 3C1 - 3C2 + P2)t³ + (3P1 - 6C1 + 3C2)t² + (-3P1 + 3C1)t + P1
    results3 cubicSolveTByY(float y) const {
        let a = -P1.y + 3.0f*C1.y - 3.0f*C2.y + P2.y;
        let b = 3.0f*P1.y - 6.0f*C1.y + 3.0f*C2.y;
        let c = -3.0f*P1.y + 3.0f*C1.y;
        let d = P1.y;
        return solveCubic(a, b, c, d - y);
    }

    results1 linearSolveXByY(float y) const {
        if (P1.y == P2.y) {
            // Ignore horizontal lines.
            return {};
        }

        let height = P2.y - P1.y;

        let t = (y - P1.y) / height;
        if (t >= 0.0f && t < 1.0f) {
            let width = P2.x - P1.x;
            return {width * t + P1.x};
        } else {
            return {};
        }
    }

    results2 quadraticSolveXByY(float y) const {
        if (y < std::min({P1.y, C1.y, P2.y}) || y > std::max({P1.y, C1.y, P2.y})) {
            return {};
        }

        results2 r;
        for (let t: quadraticSolveTByY(y)) {
            // When two curves are sampled exactly on an end point we need to make sure we only return one answer.
            // Therefor we do not use the result of the 1.0f end-point.
            if (t >= 0.0f && t < 1.0f) {
                let a = P1.x - 2.0f * C1.x + P2.x;
                let b = 2.0f*(C1.x - P1.x);
                let c = P1.x;
                r.add(a*t*t + b*t + c);
            }
        }

        return r;
    }

    results3 cubicSolveXByY(float y) const {
        if (y < std::min({ P1.y, C1.y, C2.y, P2.y }) || y > std::max({ P1.y, C1.y, C2.y, P2.y })) {
            return {};
        }

        results3 r;
        for (let t: cubicSolveTByY(y)) {
            if (t >= 0.0f && t < 1.0f) {
                let a = -P1.x + 3.0f * C1.x - 3.0f * C2.x + P2.x;
                let b = 3.0f * P1.x - 6.0f * C1.x + 3.0f * C2.x;
                let c = -3.0f * P1.x + 3.0f * C1.x;
                let d = P1.x;
                r.add(a*t*t*t + b*t*t + c*t + d);
            }
        }

        return r;
    }

    results3 solveXByY(float y) const {
        switch (type) {
        case Type::Linear: return linearSolveXByY(y);
        case Type::Quadratic: return quadraticSolveXByY(y);
        case Type::Cubic: return cubicSolveXByY(y);
        default: no_default;
        }
    }
};

inline std::vector<float> solveCurvesXByY(std::vector<Bezier> const &v, float y) {
    std::vector<float> r;
    r.reserve(v.size());

    for (let &curve: v) {
        for (let x: curve.solveXByY(y)) {
            r.push_back(x);
        }
    }
    return r;
}

/*! Render a single row of pixels.
 * Each row needs to be rendered 5 times as slightly different heights, performing super sampling.
 * The row needs to be cleared (set to zero) before rendering it.
 * Fully covered sub-pixels will have the value 0xff;
 */
void renderRow(gsl::span<uint8_t> row, size_t rowY, std::vector<Bezier> const& curves);

}