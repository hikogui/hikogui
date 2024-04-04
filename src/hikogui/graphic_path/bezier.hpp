// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../numeric/numeric.hpp"
#include "../geometry/geometry.hpp"
#include "../container/container.hpp"
#include "../macros.hpp"
#include <hikocpu/hikocpu.hpp>
#include <array>
#include <optional>
#include <concepts>

hi_export_module(hikogui.graphic_path.bezier);

hi_export namespace hi { inline namespace v1 {

namespace detail {

template<typename T>
[[nodiscard]] constexpr T bezier_broadcast(float x) noexcept
{
    return T::broadcast(x);
}

template<>
[[nodiscard]] constexpr float bezier_broadcast(float x) noexcept
{
    return x;
}

}

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
    constexpr auto _2 = detail::bezier_broadcast<T>(2);
    return {P1 - C * _2 + P2, (C - P1) * _2, P1};
}

// B(t)=(-P_{1}+3C_{1}-3C_{2}+P_{2})t^{3}+(3P_{1}-6_{1}+3C_{2})t^{2}+(-3P_{1}+3C_{1})t+P_{1}
template<typename T>
constexpr std::array<T, 4> bezierToPolynomial(T P1, T C1, T C2, T P2) noexcept
{
    constexpr auto _3 = detail::bezier_broadcast<T>(3);
    constexpr auto _6 = detail::bezier_broadcast<T>(6);
    constexpr auto min_3 = detail::bezier_broadcast<T>(-3);
    return {-P1 + C1 * _3 - C2 * _3 + P2, P1 * _3 - C1 * _6 + C2 * _3, P1 * min_3 + C1 * _3, P1};
}

constexpr point2 bezierPointAt(point2 P1, point2 P2, float t) noexcept
{
    auto const t_ = f32x4::broadcast(t);
    auto const[a, b] = bezierToPolynomial(static_cast<f32x4>(P1), static_cast<f32x4>(P2));
    return point2{a * t_ + b};
}

constexpr point2 bezierPointAt(point2 P1, point2 C, point2 P2, float t) noexcept
{
    auto const t_ = f32x4::broadcast(t);
    auto const[a, b, c] = bezierToPolynomial(static_cast<f32x4>(P1), static_cast<f32x4>(C), static_cast<f32x4>(P2));
    return point2{a * t_ * t_ + b * t_ + c};
}

constexpr point2 bezierPointAt(point2 P1, point2 C1, point2 C2, point2 P2, float t) noexcept
{
    auto const t_ = f32x4::broadcast(t);
    auto const tt_ = t_ * t_;
    auto const ttt_ = tt_ * t_;

    auto const[a, b, c, d] =
        bezierToPolynomial(static_cast<f32x4>(P1), static_cast<f32x4>(C1), static_cast<f32x4>(C2), static_cast<f32x4>(P2));
    return point2{a * ttt_ + b * tt_ + c * t_ + d};
}

constexpr vector2 bezierTangentAt(point2 P1, point2 P2, float t) noexcept
{
    return P2 - P1;
}

constexpr vector2 bezierTangentAt(point2 P1, point2 C, point2 P2, float t) noexcept
{
    constexpr auto _2 = f32x4::broadcast(2);
    auto const t_ = f32x4::broadcast(t);
    auto const P1_ = static_cast<f32x4>(P1);
    auto const C_ = static_cast<f32x4>(C);
    auto const P2_ = static_cast<f32x4>(P2);

    return vector2{_2 * t_ * (P2_ - _2 * C_ + P1_) + _2 * (C_ - P1_)};
}

constexpr vector2 bezierTangentAt(point2 P1, point2 C1, point2 C2, point2 P2, float t) noexcept
{
    constexpr auto _2 = f32x4::broadcast(2);
    constexpr auto _3 = f32x4::broadcast(3);
    constexpr auto _6 = f32x4::broadcast(6);
    auto const t_ = f32x4::broadcast(t);
    auto const tt_ = t_ * t_;
    auto const P1_ = static_cast<f32x4>(P1);
    auto const C1_ = static_cast<f32x4>(C1);
    auto const C2_ = static_cast<f32x4>(C2);
    auto const P2_ = static_cast<f32x4>(P2);

    return vector2{_3 * tt_ * (P2_ - _3 * C2_ + _3 * C1_ - P1_) + _6 * t_ * (C2_ - _2 * C1_ + P1_) + _3 * (C1_ - P1_)};
}

hi_inline lean_vector<float> bezierFindT(float P1, float P2, float x) noexcept
{
    auto const[a, b] = bezierToPolynomial(P1, P2);
    return solvePolynomial(a, b - x);
}

hi_inline lean_vector<float> bezierFindT(float P1, float C, float P2, float x) noexcept
{
    auto const[a, b, c] = bezierToPolynomial(P1, C, P2);
    return solvePolynomial(a, b, c - x);
}

hi_inline lean_vector<float> bezierFindT(float P1, float C1, float C2, float P2, float x) noexcept
{
    auto const[a, b, c, d] = bezierToPolynomial(P1, C1, C2, P2);
    return solvePolynomial(a, b, c, d - x);
}

/** Find t on the line P1->P2 which is closest to P.
 * Used for finding the shortest distance from a point to a curve.
 * The shortest vector from a curve to a point is a normal.
 */
hi_inline lean_vector<float> bezierFindTForNormalsIntersectingPoint(point2 P1, point2 P2, point2 P) noexcept
{
    auto const t_above = dot(P - P1, P2 - P1);
    auto const t_below = dot(P2 - P1, P2 - P1);
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
hi_inline lean_vector<float> bezierFindTForNormalsIntersectingPoint(point2 P1, point2 C, point2 P2, point2 P) noexcept
{
    constexpr auto _2 = f32x4::broadcast(2);
    auto const P1_ = static_cast<f32x4>(P1);
    auto const P2_ = static_cast<f32x4>(P2);
    auto const C_ = static_cast<f32x4>(C);

    auto const p = P - P1;
    auto const p1 = C - P1;
    auto const p2 = vector2{P2_ - (_2 * C_) + P1_};

    auto const a = dot(p2, p2);
    auto const b = 3 * dot(p1, p2);
    auto const c = dot(2 * p1, p1) - dot(p2, p);
    auto const d = -dot(p1, p);
    return solvePolynomial(a, b, c, d);
}

/*! Find x for y on a bezier curve.
 * In a contour, multiple bezier curves are attached to each other
 * on the anchor point. We don't want duplicate results when
 * passing `y` that is at the same height as an anchor point.
 * So we compare with less than to the end-anchor point to remove
 * it from the result.
 */
hi_inline lean_vector<float> bezierFindX(point2 P1, point2 P2, float y) noexcept
{
    if (y < std::min({P1.y(), P2.y()}) || y > std::max({P1.y(), P2.y()})) {
        return {};
    }

    auto r = lean_vector<float>{};
    for (auto const t : bezierFindT(P1.y(), P2.y(), y)) {
        if (t >= 0.0f && t < 1.0f) {
            r.push_back(bezierPointAt(P1, P2, t).x());
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
hi_inline lean_vector<float> bezierFindX(point2 P1, point2 C, point2 P2, float y) noexcept
{
    auto r = lean_vector<float>{};

    if (y < std::min({P1.y(), C.y(), P2.y()}) || y > std::max({P1.y(), C.y(), P2.y()})) {
        return r;
    }

    for (auto const t : bezierFindT(P1.y(), C.y(), P2.y(), y)) {
        if (t >= 0.0f && t <= 1.0f) {
            r.push_back(bezierPointAt(P1, C, P2, t).x());
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
hi_inline lean_vector<float> bezierFindX(point2 P1, point2 C1, point2 C2, point2 P2, float y) noexcept
{
    auto r = lean_vector<float>{};

    if (y < std::min({P1.y(), C1.y(), C2.y(), P2.y()}) || y > std::max({P1.y(), C1.y(), C2.y(), P2.y()})) {
        return r;
    }

    for (auto const t : bezierFindT(P1.y(), C1.y(), C2.y(), P2.y(), y)) {
        if (t >= 0.0f && t <= 1.0f) {
            r.push_back(bezierPointAt(P1, C1, C2, P2, t).x());
        }
    }

    return r;
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */
hi_inline float bezierFlatness(point2 P1, point2 P2) noexcept
{
    return 1.0f;
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */
hi_inline float bezierFlatness(point2 P1, point2 C, point2 P2) noexcept
{
    auto const P1P2 = hypot(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    auto const P1C1 = hypot(C - P1);
    auto const C1P2 = hypot(P2 - C);
    return P1P2 / (P1C1 + C1P2);
}

/*! Return the flatness of a curve.
 * \return 1.0 when completely flat, < 1.0 when curved.
 */
hi_inline float bezierFlatness(point2 P1, point2 C1, point2 C2, point2 P2) noexcept
{
    auto const P1P2 = hypot(P2 - P1);
    if (P1P2 == 0.0f) {
        return 1.0;
    }

    auto const P1C1 = hypot(C1 - P1);
    auto const C1C2 = hypot(C2 - C1);
    auto const C2P2 = hypot(P2 - C2);
    return P1P2 / (P1C1 + C1C2 + C2P2);
}

hi_inline std::pair<point2, point2> parallelLine(point2 P1, point2 P2, float distance) noexcept
{
    auto const v = P2 - P1;
    auto const n = normal(v);
    return {P1 + n * distance, P2 + n * distance};
}

/*! Find the intersect points between two line segments.
 */
hi_inline std::optional<point2> getIntersectionPoint(point2 A1, point2 A2, point2 B1, point2 B2) noexcept
{
    // convert points to vectors.
    auto const p = A1;
    auto const r = A2 - A1;
    auto const q = B1;
    auto const s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    auto const crossRS = cross(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        auto const q_min_p = q - p;
        auto const t = cross(q_min_p, s) / crossRS;
        auto const u = cross(q_min_p, r) / crossRS;

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
hi_inline std::optional<point2> getExtrapolatedIntersectionPoint(point2 A1, point2 A2, point2 B1, point2 B2) noexcept
{
    // convert points to vectors.
    auto const p = A1;
    auto const r = A2 - A1;
    auto const q = B1;
    auto const s = B2 - B1;

    // find t and u in:
    // p + t*r == q + us
    auto const crossRS = cross(r, s);
    if (crossRS == 0.0f) {
        // Parallel, other non, or a range of points intersect.
        return {};

    } else {
        auto const q_min_p = q - p;
        auto const t = cross(q_min_p, s) / crossRS;

        return bezierPointAt(A1, A2, t);
    }
}

}} // namespace hi::v1
