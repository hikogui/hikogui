// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Draw/PixelMap.hpp"
#include "TTauri/Draw/attributes.hpp"
#include "TTauri/Required/math.hpp"
#include "TTauri/Foundation/bezier.hpp"
#include "TTauri/Required/required.hpp"
#include <glm/glm.hpp>
#include <tuple>
#include <limits>
#include <algorithm>

namespace TTauri::Draw {

struct BezierPoint;

/*! Quadratic Bezier Curve
*/
struct BezierCurve {
    enum class Type { None, Linear, Quadratic, Cubic };

    Type type;
    glm::vec2 P1; //!< First point
    glm::vec2 C1; //!< Control point
    glm::vec2 C2; //!< Control point
    glm::vec2 P2; //!< Last point

    BezierCurve() noexcept : type(Type::None), P1(), C1(), C2(), P2() {}

    BezierCurve(Type type, glm::vec2 P1, glm::vec2 C1, glm::vec2 C2, glm::vec2 P2) noexcept : type(type), P1(P1), C1(C1), C2(C2), P2(P2) {}

    BezierCurve(glm::vec2 P1, glm::vec2 P2) noexcept : type(Type::Linear), P1(P1), C1(), C2(), P2(P2) {}

    BezierCurve(glm::vec2 P1, glm::vec2 C1, glm::vec2 P2) noexcept : type(Type::Quadratic), P1(P1), C1(C1), C2(C1), P2(P2) {}

    BezierCurve(glm::vec2 P1, glm::vec2 C1, glm::vec2 C2, glm::vec2 P2) noexcept : type(Type::Cubic), P1(P1), C1(C1), C2(C2), P2(P2) {}

    
    glm::vec2 pointAt(float t) const noexcept {
        switch (type) {
        case Type::Linear: return bezierPointAt(P1, P2, t);
        case Type::Quadratic: return bezierPointAt(P1, C1, P2, t);
        case Type::Cubic: return bezierPointAt(P1, C1, C2, P2, t);
        default: no_default;
        }
    }

    results<float,3> solveXByY(float y) const noexcept {
        switch (type) {
        case Type::Linear: return bezierFindX(P1, P2, y);
        case Type::Quadratic: return bezierFindX(P1, C1, P2, y);
        case Type::Cubic: return bezierFindX(P1, C1, C2, P2, y);
        default: no_default;
        }
    }

    std::pair<BezierCurve,BezierCurve> cubicSplit(float t) const noexcept {
        let outerA = BezierCurve{P1, C1};
        let outerBridge = BezierCurve{C1, C2};
        let outerB = BezierCurve{C2, P2};

        let innerA = BezierCurve{outerA.pointAt(t), outerBridge.pointAt(t)};
        let innerB = BezierCurve{outerBridge.pointAt(t), outerB.pointAt(t)};

        let newPoint = BezierCurve{innerA.pointAt(t), innerB.pointAt(t)}.pointAt(t);

        return {{ P1, outerA.pointAt(t), innerA.pointAt(t), newPoint }, { newPoint, innerB.pointAt(t), outerB.pointAt(t), P2 }};
    }

    std::pair<BezierCurve,BezierCurve> quadraticSplit(float t) const noexcept {
        let outerA = BezierCurve{P1, C1};
        let outerB = BezierCurve{C1, P2};

        let newPoint = BezierCurve{outerA.pointAt(t), outerB.pointAt(t)}.pointAt(t);

        return {{ P1, outerA.pointAt(t), newPoint }, { newPoint, outerB.pointAt(t), P2 }};
    }

    std::pair<BezierCurve,BezierCurve> linearSplit(float t) const noexcept {
        let newPoint = pointAt(t);

        return {{ P1, newPoint }, { newPoint, P2 }};
    }

    std::pair<BezierCurve,BezierCurve> split(float t) const noexcept {
        switch (type) {
        case Type::Linear: return linearSplit(t);
        case Type::Quadratic: return quadraticSplit(t);
        case Type::Cubic: return cubicSplit(t);
        default: no_default;
        }
    }

    
    void subdivideUntilFlat_impl(std::vector<BezierCurve> &r, float minimumFlatness) const noexcept {
        if (flatness() >= minimumFlatness) {
            r.push_back(*this);
        } else {
            let [a, b] = split(0.5f);
            a.subdivideUntilFlat_impl(r, minimumFlatness);
            b.subdivideUntilFlat_impl(r, minimumFlatness);
        }
    }

    std::vector<BezierCurve> subdivideUntilFlat(float tolerance) const noexcept {
        std::vector<BezierCurve> r;
        subdivideUntilFlat_impl(r, 1.0f - tolerance);
        return r;
    }

    /*! Return the flatness of a curve.
    * \return 1.0 when completely flat, < 1.0 when curved.
    */
    float flatness() const noexcept {
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
    BezierCurve toParrallelLine(float offset) const noexcept {
        auto [newP1, newP2] = parrallelLine(P1, P2, offset);
        return { newP1, newP2 };
    }
};

inline bool operator==(BezierCurve const &lhs, BezierCurve const &rhs) noexcept {
    if (lhs.type != rhs.type) {
        return false;
    }
    switch (lhs.type) {
    case BezierCurve::Type::None:
        return true;
    case BezierCurve::Type::Linear:
        return (lhs.P1 == rhs.P1) && (lhs.P2 == rhs.P2);
    case BezierCurve::Type::Quadratic:
        return (lhs.P1 == rhs.P1) && (lhs.C1 == rhs.C1) && (lhs.P2 == rhs.P2);
    case BezierCurve::Type::Cubic:
        return (lhs.P1 == rhs.P1) && (lhs.C1 == rhs.C1) && (lhs.C2 == rhs.C2) && (lhs.P2 == rhs.P2);
    default:
        no_default;
    }
}


inline BezierCurve operator*(glm::mat3x3 const &lhs, BezierCurve const &rhs) noexcept {
    return {
        rhs.type,
        glm::xy(lhs * glm::vec3(rhs.P1, 1.0)),
        glm::xy(lhs * glm::vec3(rhs.C1, 1.0)),
        glm::xy(lhs * glm::vec3(rhs.C2, 1.0)),
        glm::xy(lhs * glm::vec3(rhs.P2, 1.0))
    };
}

inline BezierCurve operator*(BezierCurve const &lhs, glm::vec2 const rhs) noexcept {
    return { lhs.type, lhs.P1 * rhs, lhs.C1 * rhs, lhs.C2 * rhs, lhs.P2 * rhs };
}

inline BezierCurve &operator*=(BezierCurve &lhs, glm::vec2 const rhs) noexcept {
    lhs.P1 *= rhs;
    lhs.C1 *= rhs;
    lhs.C2 *= rhs;
    lhs.P2 *= rhs;
    return lhs;
}

inline BezierCurve operator+(BezierCurve const &lhs, glm::vec2 const rhs) noexcept {
    return { lhs.type, lhs.P1 + rhs, lhs.C1 + rhs, lhs.C2 + rhs, lhs.P2 + rhs };
}

inline BezierCurve &operator+=(BezierCurve &lhs, glm::vec2 const rhs) noexcept {
    lhs.P1 += rhs;
    lhs.C1 += rhs;
    lhs.C2 += rhs;
    lhs.P2 += rhs;
    return lhs;
}

inline BezierCurve operator~(BezierCurve const &rhs) noexcept {
    return { rhs.type, rhs.P2, rhs.C2, rhs.C1, rhs.P1 };
}

/*! Make a contour of Bezier curves from a list of points.
 */
std::vector<BezierCurve> makeContourFromPoints(std::vector<BezierPoint>::const_iterator begin, std::vector<BezierPoint>::const_iterator end) noexcept;

/*! Inverse a contour.
 */
std::vector<BezierCurve> makeInverseContour(std::vector<BezierCurve> const &contour) noexcept;

/*! Make a contour of Bezier curves from another contour of Bezier curves at a offset.
 * \offset positive means the parrallel contour will be on the starboard side of the given contour. 
 */
std::vector<BezierCurve> makeParrallelContour(std::vector<BezierCurve> const &contour, float offset, LineJoinStyle lineJoinStyle, float tolerance) noexcept;

/*! Fill a linear greyscale image by filling a curve with anti-aliasing.
 */
void fill(PixelMap<uint8_t>& image, std::vector<BezierCurve> const& curves) noexcept;

}
