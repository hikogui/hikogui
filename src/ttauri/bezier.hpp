// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "polynomial.hpp"
#include "numeric_array.hpp"
#include "mat.hpp"
#include <array>
#include <optional>

namespace tt {

// B(t)=(P_{2}-P_{1})t+P_{1}
template<typename T>
inline std::array<T, 2> bezierToPolynomial(T P1, T P2) noexcept
{
    return {P2 - P1, P1};
}

// B(t)=(P_{1}-2C+P_{2})t^{2}+2(C-P_{1})t+P_{1}
template<typename T>
inline std::array<T, 3> bezierToPolynomial(T P1, T C, T P2) noexcept
{
    return {P1 - C * 2 + P2, (C - P1) * 2, P1};
}

// B(t)=(-P_{1}+3C_{1}-3C_{2}+P_{2})t^{3}+(3P_{1}-6_{1}+3C_{2})t^{2}+(-3P_{1}+3C_{1})t+P_{1}
template<typename T>
inline std::array<T, 4> bezierToPolynomial(T P1, T C1, T C2, T P2) noexcept
{
    return {-P1 + C1 * 3 - C2 * 3 + P2, P1 * 3 - C1 * 6 + C2 * 3, P1 * -3 + C1 * 3, P1};
}

inline f32x4 bezierPointAt(f32x4 P1, f32x4 P2, float t) noexcept
{
    tt_axiom(P1.is_point() && P2.is_point());

    ttlet[a, b] = bezierToPolynomial(P1, P2);
    return a * t + b;
}

inline f32x4 bezierPointAt(f32x4 P1, f32x4 C, f32x4 P2, float t) noexcept
{
    tt_axiom(P1.is_point() && C.is_point() && P2.is_point());

    ttlet[a, b, c] = bezierToPolynomial(P1, C, P2);
    return a * t * t + b * t + c;
}

inline f32x4 bezierPointAt(f32x4 P1, f32x4 C1, f32x4 C2, f32x4 P2, float t) noexcept
{
    tt_axiom(P1.is_point() && C1.is_point() && C2.is_point() && P2.is_point());

    ttlet[a, b, c, d] = bezierToPolynomial(P1, C1, C2, P2);
    return a * t * t * t + b * t * t + c * t + d;
}

inline f32x4 bezierTangentAt(f32x4 P1, f32x4 P2, float t) noexcept
{
    tt_axiom(P1.is_point() && P2.is_point());
    return P2 - P1;
}

inline f32x4 bezierTangentAt(f32x4 P1, f32x4 C, f32x4 P2, float t) noexcept
{
    tt_axiom(P1.is_point() && C.is_point() && P2.is_point());
    return 2 * t * (P2 - 2 * C + P1) + 2 * (C - P1);
}

inline f32x4 bezierTangentAt(f32x4 P1, f32x4 C1, f32x4 C2, f32x4 P2, float t) noexcept
{
    tt_axiom(P1.is_point() && C1.is_point() && C2.is_point() && P2.is_point());
    return 3 * t * t * (P2 - 3 * C2 + 3 * C1 - P1) + 6 * t * (C2 - 2 * C1 + P1) + 3 * (C1 - P1);
}

inline results<float, 1> bezierFindT(float P1, float P2, float x) noexcept
{
    ttlet[a, b] = bezierToPolynomial(P1, P2);
    return solvePolynomial(a, b - x);
}

inline results<float, 2> bezierFindT(float P1, float C, float P2, float x) noexcept
{
    ttlet[a, b, c] = bezierToPolynomial(P1, C, P2);
    return solvePolynomial(a, b, c - x);
}

inline results<float, 3> bezierFindT(float P1, float C1, float C2, float P2, float x) noexcept
{
    ttlet[a, b, c, d] = bezierToPolynomial(P1, C1, C2, P2);
    return solvePolynomial(a, b, c, d - x);
}

/** Find t on the line P1->P2 which is closest to P.
 * Used for finding the shortest distance from a point to a curve.
 * The shortest vector from a curve to a point is a normal.
 */
inline results<float, 1> bezierFindTForNormalsIntersectingPoint(f32x4 P1, f32x4 P2, f32x4 P) noexcept
{
    tt_axiom(P1.is_point() && P2.is_point() && P.is_point());

    auto t_above = dot<2>(P - P1, P2 - P1);
    auto t_below = dot<2>(P2 - P1, P2 - P1);
    if (t_below == 0.0) {
        [[unlikely]] return {};
    } else {
        return {t_above / t_below};
    }
}

/** Find t on the curve P1->C->P2 which is closest to P.
 * Used for finding the shortest distance from a point to a curve.
 * The shortest vector from a curve to a point is a normal.
 */
inline results<float, 3> bezierFindTForNormalsIntersectingPoint(f32x4 P1, f32x4 C, f32x4 P2, f32x4 P) noexcept
{
    tt_axiom(P1.is_point() && C.is_point() && P2.is_point() && P.is_point());

    ttlet p = P - P1;
    ttlet p1 = C - P1;
    ttlet p2 = P2 - (2 * C) + P1;

    ttlet a = dot<2>(p2, p2);
    ttlet b = 3 * dot<2>(p1, p2);
    ttlet c = dot<2>(2 * p1, p1) - dot<2>(p2, p);
    ttlet d = -dot<2>(p1, p);
    return solvePolynomial(a, b, c, d);
}

/*! Find x for y on a bezier curve.
 * In a contour, multiple bezier curves are attached to each other
 * on the anchor point. We don't want duplicate results when
 * passing `y` that is at the same height as an anchor point.
 * So we compare with less than to the end-anchor point to remove
 * it from the result.
 */
inline results<float, 1> bezierFindX(f32x4 P1, f32x4 P2, float y) noexcept
{
    tt_axiom(P1.is_point() && P2.is_point());

    if (y < std::min({P1.y(), P2.y()}) || y > std::max({P1.y(), P2.y()})) {
        return {};
    }

    results<float, 1> r;
    for (ttlet t : bezierFindT(P1.y(), P2.y(), y)) {
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
inline results<float, 2> bezierFindX(f32x4 P1, f32x4 C, f32x4 P2, float y) noexcept
{
    tt_axiom(P1.is_point() && C.is_point() && P2.is_point());

    results<float, 2> r{};

    if (y < std::min({P1.y(), C.y(), P2.y()}) || y > std::max({P1.y(), C.y(), P2.y()})) {
        return r;
    }

    for (ttlet t : bezierFindT(P1.y(), C.y(), P2.y(), y)) {
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
inline results<float, 3> bezierFindX(f32x4 P1, f32x4 C1, f32x4 C2, f32x4 P2, float y) noexcept
{
    tt_axiom(P1.is_point() && C1.is_point() && C2.is_point() && P2.is_point());

    results<float, 3> r{};

    if (y < std::min({P1.y(), C1.y(), C2.y(), P2.y()}) || y > std::max({P1.y(), C1.y(), C2.y(), P2.y()})) {
        return r;
    }

    for (ttlet t : bezierFindT(P1.y(), C1.y(), C2.y(), P2.y(), y)) {
        if (t >= 0.0f && t <= 1.0f) {
            r.add(bezierPointAt(P1, C1, C2, P2, t).x());
        }
    }

    return r;
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */
inline float bezierFlatness(f32x4 P1, f32x4 P2) noexcept
{
    tt_axiom(P1.is_point() && P2.is_point());

    return 1.0f;
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */

inline float bezierFlatness(f32x4 P1, f32x4 C, f32x4 P2) noexcept
{
    tt_axiom(P1.is_point() && C.is_point() && P2.is_point());

    ttlet P1P2 = hypot<2>(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    ttlet P1C1 = hypot<2>(C - P1);
    ttlet C1P2 = hypot<2>(P2 - C);
    return P1P2 / (P1C1 + C1P2);
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */

inline float bezierFlatness(f32x4 P1, f32x4 C1, f32x4 C2, f32x4 P2) noexcept
{
    tt_axiom(P1.is_point() && C1.is_point() && C2.is_point() && P2.is_point());

    ttlet P1P2 = hypot<2>(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    ttlet P1C1 = hypot<2>(C1 - P1);
    ttlet C1C2 = hypot<2>(C2 - C1);
    ttlet C2P2 = hypot<2>(P2 - C2);
    return P1P2 / (P1C1 + C1C2 + C2P2);
}

inline std::pair<f32x4, f32x4> parrallelLine(f32x4 P1, f32x4 P2, float distance) noexcept
{
    tt_axiom(P1.is_point() && P2.is_point());

    ttlet v = P2 - P1;
    ttlet n = normal<2>(v);
    return {P1 + n * distance, P2 + n * distance};
}

/*! Find the intersect points between two line segments.
 */
inline std::optional<f32x4> getIntersectionPoint(f32x4 A1, f32x4 A2, f32x4 B1, f32x4 B2) noexcept
{
    tt_axiom(A1.is_point() && A2.is_point() && B1.is_point() && B2.is_point());

    // convert points to vectors.
    ttlet p = A1;
    ttlet r = A2 - A1;
    ttlet q = B1;
    ttlet s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    ttlet crossRS = viktor_cross<2>(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        ttlet q_min_p = q - p;
        ttlet t = viktor_cross<2>(q_min_p, s) / crossRS;
        ttlet u = viktor_cross<2>(q_min_p, r) / crossRS;

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
inline std::optional<f32x4> getExtrapolatedIntersectionPoint(f32x4 A1, f32x4 A2, f32x4 B1, f32x4 B2) noexcept
{
    tt_axiom(A1.is_point() && A2.is_point() && B1.is_point() && B2.is_point());

    // convert points to vectors.
    ttlet p = A1;
    ttlet r = A2 - A1;
    ttlet q = B1;
    ttlet s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    ttlet crossRS = viktor_cross<2>(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        ttlet q_min_p = q - p;
        ttlet t = viktor_cross<2>(q_min_p, s) / crossRS;

        return bezierPointAt(A1, A2, t);
    }
}

} // namespace tt
