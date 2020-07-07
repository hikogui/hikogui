// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/polynomial.hpp"
#include "ttauri/foundation/vec.hpp"
#include "ttauri/foundation/mat.hpp"
#include <array>
#include <optional>

namespace tt {

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
    ttlet _2 = T{2.0};
    return {
        P1 - C * _2 + P2,
        (C - P1) * _2,
        P1
    };
}

// B(t)=(-P_{1}+3C_{1}-3C_{2}+P_{2})t^{3}+(3P_{1}-6_{1}+3C_{2})t^{2}+(-3P_{1}+3C_{1})t+P_{1}
template<typename T>
inline std::array<T,4> bezierToPolynomial(T P1, T C1, T C2, T P2) noexcept
{
    ttlet _3 = T{3.0};
    ttlet min_3 = T{-3.0};
    ttlet _6 = T{6.0};
    return {
        -P1 + C1 * _3 - C2 * _3 + P2,
        P1 * _3 - C1 * _6 + C2 * _3,
        P1 * min_3 + C1 * _3,
        P1
    };
}

inline vec bezierPointAt(vec P1, vec P2, float t) noexcept {
    tt_assume(P1.w() == 1.0f && P2.w() == 1.0f);

    ttlet [a, b] = bezierToPolynomial(P1, P2);
    ttlet t_ = vec{t};
    return a * t_ + b;
}

inline vec bezierPointAt(vec P1, vec C, vec P2, float t) noexcept
{
    tt_assume(P1.w() == 1.0f && C.w() == 1.0f && P2.w() == 1.0f);

    ttlet [a, b, c] = bezierToPolynomial(P1, C, P2);

    ttlet t_ = vec{t};
    return a*t_*t_ + b*t_ + c;
}

inline vec bezierPointAt(vec P1, vec C1, vec C2, vec P2, float t) noexcept
{
    tt_assume(P1.w() == 1.0f && C1.w() == 1.0f && C2.w() && P2.w() == 1.0f);

    ttlet [a, b, c, d] = bezierToPolynomial(P1, C1, C2, P2);
    ttlet t_ = vec{t};
    return a*t_*t_*t_ + b*t_*t_ + c*t_ + d;
}

inline vec bezierTangentAt(vec P1, vec P2, float t) noexcept
{
    tt_assume(P1.w() == 1.0f && P2.w() == 1.0f);

    return P2 - P1;
}

inline vec bezierTangentAt(vec P1, vec C, vec P2, float t) noexcept
{
    tt_assume(P1.w() == 1.0f && C.w() == 1.0f && P2.w() == 1.0f);

    ttlet _2 = vec{2.0};
    ttlet _t = vec{t};
    return _2 * _t * (P2 - _2 * C + P1) + _2 * (C - P1);
} 

inline vec bezierTangentAt(vec P1, vec C1, vec C2, vec P2, float t) noexcept
{
    tt_assume(P1.w() == 1.0f && C1.w() == 1.0f && C2.w() && P2.w() == 1.0f);

    ttlet _2 = vec{2.0};
    ttlet _3 = vec{3.0};
    ttlet _6 = vec{6.0};
    ttlet _t = vec{t};
    return 
        _3 * _t * _t * (P2 - _3 * C2 + _3 * C1 - P1) +
        _6 * _t * (C2 - _2 * C1 + P1) +
        _3 * (C1 - P1);
}

inline results<float,1> bezierFindT(float P1, float P2, float x) noexcept
{
    ttlet [a, b] = bezierToPolynomial(P1, P2);
    return solvePolynomial(a, b - x);
}

inline results<float,2> bezierFindT(float P1, float C, float P2, float x) noexcept
{
    ttlet [a, b, c] = bezierToPolynomial(P1, C, P2);
    return solvePolynomial(a, b, c - x);
}

inline results<float,3> bezierFindT(float P1, float C1, float C2, float P2, float x) noexcept
{
    ttlet [a, b, c, d] = bezierToPolynomial(P1, C1, C2, P2);
    return solvePolynomial(a, b, c, d - x);
}

/** Find t on the line P1->P2 which is closest to P.
 * Used for finding the shortest distance from a point to a curve.
 * The shortest vector from a curve to a point is a normal.
 */
inline results<float,1> bezierFindTForNormalsIntersectingPoint(vec P1, vec P2, vec P) noexcept
{
    tt_assume(P1.w() == 1.0f && P2.w() == 1.0f && P.w() == 1.0f);

    auto t_above = dot(P - P1, P2 - P1);
    auto t_below = dot(P2 - P1, P2 - P1);
    if (tt_unlikely(t_below == 0.0)) {
        return {};
    } else {
        return {t_above / t_below};
    }
}

/** Find t on the curve P1->C->P2 which is closest to P.
* Used for finding the shortest distance from a point to a curve.
* The shortest vector from a curve to a point is a normal.
*/
inline results<float,3> bezierFindTForNormalsIntersectingPoint(vec P1, vec C, vec P2, vec P) noexcept
{
    tt_assume(P1.w() == 1.0f && C.w() == 1.0f && P2.w() == 1.0f && P.w() == 1.0f);

    ttlet _2 = vec{2.0};
    ttlet p = P - P1;
    ttlet p1 = C - P1;
    ttlet p2 = P2 - (_2 * C) + P1;

    ttlet a = dot(p2, p2);
    ttlet b = 3.0f * dot(p1, p2);
    ttlet c = dot(_2 * p1, p1) - dot(p2, p);
    ttlet d = -dot(p1, p);
    return solvePolynomial(a, b, c, d);
}

/*! Find x for y on a bezier curve.
 * In a contour, multiple bezier curves are attached to each other
 * on the anchor point. We don't want duplicate results when
 * passing `y` that is at the same height as an anchor point.
 * So we compare with less than to the end-anchor point to remove
 * it from the result.
 */
inline results<float,1> bezierFindX(vec P1, vec P2, float y) noexcept
{
    tt_assume(P1.w() == 1.0f && P2.w() == 1.0f);

    if (y < std::min({P1.y(), P2.y()}) || y > std::max({P1.y(), P2.y()})) {
        return {};
    }

    results<float,1> r;
    for (ttlet t: bezierFindT(P1.y(), P2.y(), y)) {
        if (t >= 0.0f && t < 1.0f) {
            r.add(bezierPointAt(P1, P2, t).x());
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
inline results<float,2> bezierFindX(vec P1, vec C, vec P2, float y) noexcept
{
    tt_assume(P1.w() == 1.0f && C.w() == 1.0f && P2.w() == 1.0f);

    results<float,2> r{};

    if (y < std::min({P1.y(), C.y(), P2.y()}) || y > std::max({P1.y(), C.y(), P2.y()})) {
        return r;
    }

    for (ttlet t: bezierFindT(P1.y(), C.y(), P2.y(), y)) {
        if (t >= 0.0f && t <= 1.0f) {
            r.add(bezierPointAt(P1, C, P2, t).x());
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
inline results<float,3> bezierFindX(vec P1, vec C1, vec C2, vec P2, float y) noexcept
{
    tt_assume(P1.w() == 1.0f && C1.w() == 1.0f && C2.w() == 1.0f && P2.w() == 1.0f);

    results<float,3> r{};

    if (y < std::min({ P1.y(), C1.y(), C2.y(), P2.y() }) || y > std::max({ P1.y(), C1.y(), C2.y(), P2.y() })) {
        return r;
    }

    for (ttlet t: bezierFindT(P1.y(), C1.y(), C2.y(), P2.y(), y)) {
        if (t >= 0.0f && t <= 1.0f) {
            r.add(bezierPointAt(P1, C1, C2, P2, t).x());
        }
    }

    return r;
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/
inline float bezierFlatness(vec P1, vec P2) noexcept
{
    tt_assume(P1.w() == 1.0f && P2.w() == 1.0f);

    return 1.0f;
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/

inline float bezierFlatness(vec P1, vec C, vec P2) noexcept
{
    tt_assume(P1.w() == 1.0f && C.w() == 1.0f && P2.w() == 1.0f);

    ttlet P1P2 = length(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    ttlet P1C1 = length(C - P1);
    ttlet C1P2 = length(P2 - C);
    return P1P2 / (P1C1 + C1P2);
}

/*! Return the flatness of a curve.
* \return 1.0 when completely flat, < 1.0 when curved.
*/

inline float bezierFlatness(vec P1, vec C1, vec C2, vec P2) noexcept
{
    tt_assume(P1.w() == 1.0f && C1.w() == 1.0f && C2.w() == 1.0f && P2.w() == 1.0f);

    ttlet P1P2 = length(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    ttlet P1C1 = length(C1 - P1);
    ttlet C1C2 = length(C2 - C1);
    ttlet C2P2 = length(P2 - C2);
    return P1P2 / (P1C1 + C1C2 + C2P2);
}

inline std::pair<vec, vec> parrallelLine(vec P1, vec P2, float distance) noexcept
{
    tt_assume(P1.w() == 1.0f && P2.w() == 1.0f);

    ttlet _distance = vec{distance};
    ttlet v = P2 - P1;
    ttlet n = normal(v);
    return {
        P1 + n * _distance,
        P2 + n * _distance
    };
}

/*! Find the intersect points between two line segments.
*/
inline std::optional<vec> getIntersectionPoint(vec A1, vec A2, vec B1, vec B2) noexcept
{
    tt_assume(A1.w() == 1.0f && A2.w() == 1.0f && B1.w() == 1.0f && B2.w() == 1.0f);

    // convert points to vectors.
    ttlet p = A1;
    ttlet r = A2 - A1;
    ttlet q = B1;
    ttlet s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    ttlet crossRS = viktor_cross(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        ttlet q_min_p = q - p;
        ttlet t = viktor_cross(q_min_p, s) / crossRS;
        ttlet u = viktor_cross(q_min_p, r) / crossRS;

        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            return bezierPointAt(A1, A2, t);
        } else {
            // The lines intersect outside of one or both of the segments.
            return {};
        }
    }
}

/*! Find the intersect points between two line segments.
*/
inline std::optional<vec> getExtrapolatedIntersectionPoint(vec A1, vec A2, vec B1, vec B2) noexcept
{
    tt_assume(A1.w() == 1.0f && A2.w() == 1.0f && B1.w() == 1.0f && B2.w() == 1.0f);

    // convert points to vectors.
    ttlet p = A1;
    ttlet r = A2 - A1;
    ttlet q = B1;
    ttlet s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    ttlet crossRS = viktor_cross(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        ttlet q_min_p = q - p;
        ttlet t = viktor_cross(q_min_p, s) / crossRS;

        return bezierPointAt(A1, A2, t);
    }
}

}
