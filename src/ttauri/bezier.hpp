// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "polynomial.hpp"
#include "geometry/numeric_array.hpp"
#include "geometry/point.hpp"
#include <array>
#include <optional>

namespace tt {

// B(t)=(P_{2}-P_{1})t+P_{1}
template<typename T>
constexpr std::array<T, 2> bezierToPolynomial(T P1, T P2) noexcept
{
    return {P2 - P1, P1};
}

// B(t)=(P_{1}-2C+P_{2})t^{2}+2(C-P_{1})t+P_{1}
template<typename T>
constexpr std::array<T, 3> bezierToPolynomial(T P1, T C, T P2) noexcept
{
    return {P1 - C * 2 + P2, (C - P1) * 2, P1};
}

// B(t)=(-P_{1}+3C_{1}-3C_{2}+P_{2})t^{3}+(3P_{1}-6_{1}+3C_{2})t^{2}+(-3P_{1}+3C_{1})t+P_{1}
template<typename T>
constexpr std::array<T, 4> bezierToPolynomial(T P1, T C1, T C2, T P2) noexcept
{
    return {-P1 + C1 * 3 - C2 * 3 + P2, P1 * 3 - C1 * 6 + C2 * 3, P1 * -3 + C1 * 3, P1};
}

template<int D>
constexpr geo::point<D> bezierPointAt(geo::point<D> P1, geo::point<D> P2, float t) noexcept
{
    ttlet[a, b] = bezierToPolynomial(static_cast<f32x4>(P1), static_cast<f32x4>(P2));
    return geo::point<D>{a * t + b};
}

template<int D>
constexpr geo::point<D> bezierPointAt(geo::point<D> P1, geo::point<D> C, geo::point<D> P2, float t) noexcept
{
    ttlet[a, b, c] = bezierToPolynomial(static_cast<f32x4>(P1), static_cast<f32x4>(C), static_cast<f32x4>(P2));
    return geo::point<D>{a * t * t + b * t + c};
}

template<int D>
constexpr geo::point<D> bezierPointAt(geo::point<D> P1, geo::point<D> C1, geo::point<D> C2, geo::point<D> P2, float t) noexcept
{
    ttlet[a, b, c, d] =
        bezierToPolynomial(static_cast<f32x4>(P1), static_cast<f32x4>(C1), static_cast<f32x4>(C2), static_cast<f32x4>(P2));
    return geo::point<D>{a * t * t * t + b * t * t + c * t + d};
}

template<int D>
inline geo::vector<D> bezierTangentAt(geo::point<D> P1, geo::point<D> P2, float t) noexcept
{
    return P2 - P1;
}

template<int D>
inline geo::vector<D> bezierTangentAt(geo::point<D> P1, geo::point<D> C, geo::point<D> P2, float t) noexcept
{
    ttlet P1_ = static_cast<f32x4>(P1);
    ttlet C_ = static_cast<f32x4>(C);
    ttlet P2_ = static_cast<f32x4>(P2);

    return geo::vector<D>{2 * t * (P2_ - 2 * C_ + P1_) + 2 * (C_ - P1_)};
}

template<int D>
inline geo::vector<D> bezierTangentAt(geo::point<D> P1, geo::point<D> C1, geo::point<D> C2, geo::point<D> P2, float t) noexcept
{
    ttlet P1_ = static_cast<f32x4>(P1);
    ttlet C1_ = static_cast<f32x4>(C1);
    ttlet C2_ = static_cast<f32x4>(C2);
    ttlet P2_ = static_cast<f32x4>(P2);

    return geo::vector<D>{3 * t * t * (P2_ - 3 * C2_ + 3 * C1_ - P1_) + 6 * t * (C2_ - 2 * C1_ + P1_) + 3 * (C1_ - P1_)};
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
inline results<float, 1> bezierFindTForNormalsIntersectingPoint(point2 P1, point2 P2, point2 P) noexcept
{
    auto t_above = dot(P - P1, P2 - P1);
    auto t_below = dot(P2 - P1, P2 - P1);
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
inline results<float, 3> bezierFindTForNormalsIntersectingPoint(point2 P1, point2 C, point2 P2, point2 P) noexcept
{
    ttlet P1_ = static_cast<f32x4>(P1);
    ttlet P2_ = static_cast<f32x4>(P2);
    ttlet C_ = static_cast<f32x4>(C);

    ttlet p = P - P1;
    ttlet p1 = C - P1;
    ttlet p2 = vector2{P2_ - (2 * C_) + P1_};

    ttlet a = dot(p2, p2);
    ttlet b = 3 * dot(p1, p2);
    ttlet c = dot(2 * p1, p1) - dot(p2, p);
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
inline results<float, 1> bezierFindX(point2 P1, point2 P2, float y) noexcept
{
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
inline results<float, 2> bezierFindX(point2 P1, point2 C, point2 P2, float y) noexcept
{
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
inline results<float, 3> bezierFindX(point2 P1, point2 C1, point2 C2, point2 P2, float y) noexcept
{
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
inline float bezierFlatness(point2 P1, point2 P2) noexcept
{
    return 1.0f;
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */

inline float bezierFlatness(point2 P1, point2 C, point2 P2) noexcept
{
    ttlet P1P2 = hypot(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    ttlet P1C1 = hypot(C - P1);
    ttlet C1P2 = hypot(P2 - C);
    return P1P2 / (P1C1 + C1P2);
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */

inline float bezierFlatness(point2 P1, point2 C1, point2 C2, point2 P2) noexcept
{
    ttlet P1P2 = hypot(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    ttlet P1C1 = hypot(C1 - P1);
    ttlet C1C2 = hypot(C2 - C1);
    ttlet C2P2 = hypot(P2 - C2);
    return P1P2 / (P1C1 + C1C2 + C2P2);
}

inline std::pair<point2, point2> parrallelLine(point2 P1, point2 P2, float distance) noexcept
{
    ttlet v = P2 - P1;
    ttlet n = normal(v);
    return {P1 + n * distance, P2 + n * distance};
}

/*! Find the intersect points between two line segments.
 */
inline std::optional<point2> getIntersectionPoint(point2 A1, point2 A2, point2 B1, point2 B2) noexcept
{
    // convert points to vectors.
    ttlet p = A1;
    ttlet r = A2 - A1;
    ttlet q = B1;
    ttlet s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    ttlet crossRS = cross(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        ttlet q_min_p = q - p;
        ttlet t = cross(q_min_p, s) / crossRS;
        ttlet u = cross(q_min_p, r) / crossRS;

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
inline std::optional<point2> getExtrapolatedIntersectionPoint(point2 A1, point2 A2, point2 B1, point2 B2) noexcept
{
    // convert points to vectors.
    ttlet p = A1;
    ttlet r = A2 - A1;
    ttlet q = B1;
    ttlet s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    ttlet crossRS = cross(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        ttlet q_min_p = q - p;
        ttlet t = cross(q_min_p, s) / crossRS;

        return bezierPointAt(A1, A2, t);
    }
}

} // namespace tt
