// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"
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

    Bezier(Type type, glm::vec2 P1, glm::vec2 C1, glm::vec2 C2, glm::vec2 P2) : type(type), P1(P1), C1(C1), C2(C2), P2(P2) {}

    Bezier(glm::vec2 P1, glm::vec2 P2) : type(Type::Linear), P1(P1), C1(), C2(), P2(P2) {}

    Bezier(glm::vec2 P1, glm::vec2 C1, glm::vec2 P2) : type(Type::Quadratic), P1(P1), C1(C1), C2(C1), P2(P2) {}

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

    Bezier operator*(glm::mat3x3 const &M) const {
        return {
            type,
            (M * glm::vec3(P1, 1.0)).xy(),
            (M * glm::vec3(C1, 1.0)).xy(),
            (M * glm::vec3(C2, 1.0)).xy(),
            (M * glm::vec3(P2, 1.0)).xy()
        };
    }

    Bezier &operator*=(glm::mat3x3 const &M) {
        P1 = (M * glm::vec3(P1, 1.0)).xy();
        C1 = (M * glm::vec3(C1, 1.0)).xy();
        C2 = (M * glm::vec3(C2, 1.0)).xy();
        P2 = (M * glm::vec3(P2, 1.0)).xy();
        return *this;
    }

    Bezier operator*(glm::vec2 const s) const {
        return { type, P1 * s, C1 * s, C2 * s, P2 * s };
    }

    Bezier &operator*=(glm::vec2 const s) {
        P1 *= s;
        C1 *= s;
        C2 *= s;
        P2 *= s;
        return *this;
    }

    Bezier operator+(glm::vec2 const t) const {
        return { type, P1 + t, C1 + t, C2 + t, P2 + t };
    }

    Bezier operator+=(glm::vec2 const t) {
        P1 += t;
        C1 += t;
        C2 += t;
        P2 += t;
    }

    Bezier operator~() const {
        return { type, P2, C2, C1, P1 };
    }

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

    glm::vec2 pointAt(float t) const {
        switch (type) {
        case Type::Linear: return bezierPointAt(P1, P2, t);
        case Type::Quadratic: return bezierPointAt(P1, C1, P2, t);
        case Type::Cubic: return bezierPointAt(P1, C1, C2, P2, t);
        default: no_default;
        }
    }

    results1 linearSolveXByY(float y) const {
        if (P1.y == P2.y) {
            // Ignore horizontal lines.
            return {};
        }

        let height = P2.y - P1.y;

        let t = (y - P1.y) / height;
        if (t >= 0.0f && t <= 1.0f) {
            return bezierPointAt(P1, P2, t).x;
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
            if (t >= 0.0f && t <= 1.0f) {
                r.add(bezierPointAt(P1, C1, P2, t).x);
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
            if (t >= 0.0f && t <= 1.0f) {
                r.add(bezierPointAt(P1, C1, C2, P2, t).x);
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

    std::pair<Bezier,Bezier> cubicSplit(float t) const {
        let outerA = Bezier{P1, C1};
        let outerBridge = Bezier{C1, C2};
        let outerB = Bezier{C2, P2};

        let innerA = Bezier{outerA.pointAt(t), outerBridge.pointAt(t)};
        let innerB = Bezier{outerBridge.pointAt(t), outerB.pointAt(t)};

        let newPoint = Bezier{innerA.pointAt(t), innerB.pointAt(t)}.pointAt(t);

        return {{ P1, outerA.pointAt(t), innerA.pointAt(t), newPoint }, { newPoint, innerB.pointAt(t), outerB.pointAt(t), P2 }};
    }

    std::pair<Bezier,Bezier> quadraticSplit(float t) const {
        let outerA = Bezier{P1, C1};
        let outerB = Bezier{C1, P2};

        let newPoint = Bezier{outerA.pointAt(t), outerB.pointAt(t)}.pointAt(t);

        return {{ P1, outerA.pointAt(t), newPoint }, { newPoint, outerB.pointAt(t), P2 }};
    }

    std::pair<Bezier,Bezier> linearSplit(float t) const {
        let newPoint = pointAt(t);

        return {{ P1, newPoint }, { newPoint, P2 }};
    }

    std::pair<Bezier,Bezier> split(float t) const {
        switch (type) {
        case Type::Linear: return linearSplit(t);
        case Type::Quadratic: return quadraticSplit(t);
        case Type::Cubic: return cubicSplit(t);
        default: no_default;
        }
    }

    void subdivideUntilFlat_impl(std::vector<Bezier> &r, float minimumFlatness) const {
        if (flatness() >= minimumFlatness) {
            r.push_back(*this);
        } else {
            let [a, b] = split(0.5f);
            a.subdivideUntilFlat_impl(r, minimumFlatness);
            b.subdivideUntilFlat_impl(r, minimumFlatness);
        }
    }

    std::vector<Bezier> subdivideUntilFlat(float tolerance) const {
        std::vector<Bezier> r;
        subdivideUntilFlat_impl(r, 1.0f - tolerance);
        return r;
    }

    /*! Return the flatness of a curve.
    * \return 1.0 when completely flat, < 1.0 when curved.
    */
    float flatness() const {
        switch (type) {
        case Type::Linear: return bezierFlatness(P1, P2);
        case Type::Quadratic: return bezierFlatness(P1, C1, P2);
        case Type::Cubic: return bezierFlatness(P1, C1, C2, P2);
        default: no_default;
        }
    }

    /*! Return a line-segment from a curve at a certain distance.
     * \offset positive means the parrallel line will be on the starboard of the curve.
     */
    Bezier toParrallelLine(float offset) const {
        auto [newP1, newP2] = parrallelLine(P1, P2, offset);
        return { newP1, newP2 };
    }
};

/*! Make a contour of Bezier curves from a list of points.
 */
std::vector<Bezier> makeContourFromPoints(std::vector<BezierPoint> const& points);

/*! Inverse a contour.
 */
std::vector<Bezier> makeInverseContour(std::vector<Bezier> const &contour);

/*! Make a contour of Bezier curves from another contour of Bezier curves at a offset.
 * \offset positive means the parrallel contour will be on the starboard side of the given contour. 
 */
std::vector<Bezier> makeParrallelContour(std::vector<Bezier> const &contour, float offset, float tolerance);

/*! Fill a linear greyscale image by filling a curve with anti-aliasing.
 */
void fill(PixelMap<uint8_t>& image, std::vector<Bezier> const& curves);

}