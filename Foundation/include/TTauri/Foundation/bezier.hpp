// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/polynomial.hpp"
#include "TTauri/Foundation/geometry.hpp"

namespace TTauri {

// B(t)=(P_{2}-P_{1})t+P_{1}
template<typename T>
inline std::array<T,2> bezierToPolynomial(T P1, T P2) noexcept
{
    return {
        P2 - P1,
        P1
    };
}

// B(t)=(P_{1}-2C+P_{2})t^{2}+2(C-P_{1})t+P_{1}
template<typename T>
inline std::array<T,3> bezierToPolynomial(T P1, T C, T P2) noexcept
{
    return {
        P1 - C * 2.0f + P2,
        (C - P1) * 2.0f,
        P1
    };
}

// B(t)=(-P_{1}+3C_{1}-3C_{2}+P_{2})t^{3}+(3P_{1}-6_{1}+3C_{2})t^{2}+(-3P_{1}+3C_{1})t+P_{1}
template<typename T>
inline std::array<T,4> bezierToPolynomial(T P1, T C1, T C2, T P2) noexcept
{
    return {
        -P1 + C1 * 3.0f - C2 * 3.0f + P2,
        P1 * 3.0f - C1 * 6.0f + C2 * 3.0f,
        P1 * -3.0f + C1 * 3.0f,
        P1
    };
}

template<typename T, typename U>
inline T bezierPointAt(T P1, T P2, U t) noexcept {
    let [a, b] = bezierToPolynomial(P1, P2);
    return a*t + b;
}

template<typename T, typename U>
inline T bezierPointAt(T P1, T C, T P2, U t) noexcept
{
    let [a, b, c] = bezierToPolynomial(P1, C, P2);
    return a*t*t + b*t + c;
}

template<typename T, typename U>
inline T bezierPointAt(T P1, T C1, T C2, T P2, U t) noexcept
{
    let [a, b, c, d] = bezierToPolynomial(P1, C1, C2, P2);
    return a*t*t*t + b*t*t + c*t + d;
}

template<typename T>
inline results<T,1> bezierFindT(T P1, T P2, T x) noexcept
{
    let [a, b] = bezierToPolynomial(P1, P2);
    return solvePolynomial(a, b - x);
}

template<typename T>
inline results<T,2> bezierFindT(T P1, T C, T P2, T x) noexcept
{
    let [a, b, c] = bezierToPolynomial(P1, C, P2);
    return solvePolynomial(a, b, c - x);
}

template<typename T>
inline results<T,3> bezierFindT(T P1, T C1, T C2, T P2, T x) noexcept
{
    let [a, b, c, d] = bezierToPolynomial(P1, C1, C2, P2);
    return solvePolynomial(a, b, c, d - x);
}


/*! Find x for y on a bezier curve.
 * In a contour, multiple bezier curves are attached to each other
 * on the anchor point. We don't want duplicate results when
 * passing `y` that is at the same height as an anchor point.
 * So we compare with less than to the end-anchor point to remove
 * it from the result.
 */
template <typename T, typename U>
inline results<U,1> bezierFindX(T P1, T P2, U y) noexcept
{
    if (y < std::min({P1.y, P2.y}) || y > std::max({P1.y, P2.y})) {
        return {};
    }

    results<float,1> r;
    for (let t: bezierFindT(P1.y, P2.y, y)) {
        if (t >= 0 && t < 1) {
            r.add(bezierPointAt(P1.x, P2.x, t));
        }
    }

    return r;
}

/*! Find x for y on a bezier curve.
* In a contour, multiple bezier curves are attached to each other
* on the anchor point. We don't want duplicate results when
* passing `y` that is at the same height as an anchor point.
* So we compare with less than to the end-anchor point to remove
* it from the result.
*/
template <typename T, typename U>
inline results<U,2> bezierFindX(T P1, T C, T P2, U y) noexcept
{
    if (y < std::min({P1.y, C.y, P2.y}) || y > std::max({P1.y, C.y, P2.y})) {
        return {};
    }

    results<float,2> r;
    for (let t: bezierFindT(P1.y, C.y, P2.y, y)) {
        if (t >= 0 && t <= 1) {
            r.add(bezierPointAt(P1.x, C.x, P2.x, t));
        }
    }

    return r;
}

/*! Find x for y on a bezier curve.
* In a contour, multiple bezier curves are attached to each other
* on the anchor point. We don't want duplicate results when
* passing `y` that is at the same height as an anchor point.
* So we compare with less than to the end-anchor point to remove
* it from the result.
*/
template <typename T, typename U>
results<U,3> bezierFindX(T P1, T C1, T C2, T P2, U y) noexcept
{
    if (y < std::min({ P1.y, C1.y, C2.y, P2.y }) || y > std::max({ P1.y, C1.y, C2.y, P2.y })) {
        return {};
    }

    results<float,3> r;
    for (let t: bezierFindT(P1.y, C1.y, C2.y, P2.y, y)) {
        if (t >= 0 && t <= 1) {
            r.add(bezierPointAt(P1.x, C1.x, C2.x, P2.x, t));
        }
    }

    return r;
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/
inline float bezierFlatness(glm::vec2 P1, glm::vec2 P2) noexcept
{
    return 1.0f;
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/

inline float bezierFlatness(glm::vec2 P1, glm::vec2 C, glm::vec2 P2) noexcept
{
    let P1P2 = glm::length(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    let P1C1 = glm::length(C - P1);
    let C1P2 = glm::length(P2 - C);
    return P1P2 / (P1C1 + C1P2);
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/

inline float bezierFlatness(glm::vec2 P1, glm::vec2 C1, glm::vec2 C2, glm::vec2 P2) noexcept
{
    let P1P2 = glm::length(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    let P1C1 = glm::length(C1 - P1);
    let C1C2 = glm::length(C2 - C1);
    let C2P2 = glm::length(P2 - C2);
    return P1P2 / (P1C1 + C1C2 + C2P2);
}

inline std::pair<glm::vec2, glm::vec2> parrallelLine(glm::vec2 P1, glm::vec2 P2, float distance) noexcept
{
    let v = P2 - P1;
    let n = normal(v);
    return {
        P1 + n * distance,
        P2 + n * distance
    };
}

/*! Find the intersect points between two line segments.
*/
inline std::optional<glm::vec2> getIntersectionPoint(glm::vec2 A1, glm::vec2 A2, glm::vec2 B1, glm::vec2 B2) noexcept
{
    // convert points to vectors.
    let p = A1;
    let r = A2 - A1;
    let q = B1;
    let s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    let crossRS = viktorCross(r, s);
    if (crossRS == 0.0f) {
        // Parrallel, other non, or a range of points intersect.
        return {};

    } else {
        let q_min_p = q - p;
        let t = viktorCross(q_min_p, s) / crossRS;
        let u = viktorCross(q_min_p, r) / crossRS;

        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            return bezierPointAt(A1, A2, t);
        } else {
            // The lines intesect outside of one or both of the segments.
            return {};
        }
    }
}

/*! Find the intersect points between two line segments.
*/
inline std::optional<glm::vec2> getExtrapolatedIntersectionPoint(glm::vec2 A1, glm::vec2 A2, glm::vec2 B1, glm::vec2 B2) noexcept
{
    // convert points to vectors.
    let p = A1;
    let r = A2 - A1;
    let q = B1;
    let s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    let crossRS = viktorCross(r, s);
    if (crossRS == 0.0f) {
        // Parrallel, other non, or a range of points intersect.
        return {};

    } else {
        let q_min_p = q - p;
        let t = viktorCross(q_min_p, s) / crossRS;

        return bezierPointAt(A1, A2, t);
    }
}

}
