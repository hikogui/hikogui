// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../container/module.hpp"
#include "../macros.hpp"
#include <numbers>
#include <array>

namespace hi::inline v1 {

/*! Solve line function.
 * ax+b=0
 *
 * \f[
 *  x=
 *  \begin{cases}
 *   \frac{-b}{a}, & \text{if } a\ne0\\
 *   \in\mathbb{R}, & \text{if } a=0,b=0\\
 *   \o & \text{if } a=0,b\ne0
 *  \end{cases}
 * \f]
 */
template<typename T>
hi_force_inline constexpr lean_vector<T> solvePolynomial(T const& a, T const& b) noexcept
{
    if (a != 0) {
        return {-(b / a)};
    } else if (b == 0) {
        // Any value of x is correct.
        return {0.0f};
    } else {
        // None of the values for x is correct.
        return {};
    }
}

/*! Solve quadratic function.
 * \f[ax^{2}+bx+c=0\f]
 *
 * \f[D=b^{2}-4ac\f]
 * \f[
 *  x=
 *   \begin{cases}
 *    \frac{-b}{2a}, & \text{if } D=0\\
 *    \frac{-b -\sqrt{D}}{2a}, \frac{-b +\sqrt{D}}{2a} & \text{if } D>0\\
 *    \o & \text{if } D<0
 *   \end{cases}
 * \f]
 */
template<typename T>
hi_force_inline constexpr lean_vector<T> solvePolynomial(T const& a, T const& b, T const& c) noexcept
{
    if (a == 0) {
        return solvePolynomial(b, c);
    } else {
        hilet D = b * b - static_cast<T>(4) * a * c;
        if (D < 0) {
            return {};
        } else if (D == 0) {
            return {-b / (static_cast<T>(2) * a)};
        } else {
            hilet Dsqrt = sqrt(D);
            return {(-b - Dsqrt) / (static_cast<T>(2) * a), (-b + Dsqrt) / (static_cast<T>(2) * a)};
        }
    }
}

/*! Trigonometric solution for three real roots
 */
template<typename T>
hi_force_inline lean_vector<T> solveDepressedCubicTrig(T const& p, T const& q) noexcept
{
    constexpr T oneThird = static_cast<T>(1) / static_cast<T>(3);
    constexpr T pi2_3 = (static_cast<T>(2) / static_cast<T>(3)) * std::numbers::pi_v<T>;
    constexpr T pi4_3 = (static_cast<T>(4) / static_cast<T>(3)) * std::numbers::pi_v<T>;

    hilet U = oneThird * acos(((static_cast<T>(3) * q) / (static_cast<T>(2) * p)) * sqrt(static_cast<T>(-3) / p));
    hilet V = static_cast<T>(2) * sqrt(-oneThird * p);

    hilet t0 = V * cos(U);
    hilet t1 = V * cos(U - pi2_3);
    hilet t2 = V * cos(U - pi4_3);
    return {t0, t1, t2};
}

template<typename T>
hi_force_inline lean_vector<T> solveDepressedCubicCardano(T const& p, T const& q, T const& D) noexcept
{
    hilet sqrtD = sqrt(D);
    hilet minusHalfQ = static_cast<T>(-0.5) * q;
    hilet v = cbrt(minusHalfQ + sqrtD);
    hilet w = cbrt(minusHalfQ - sqrtD);
    return {v + w};
}

/*! Solve cubic function in the form.
 * \f[t^{3}+pt+q=0\f]
 *
 * \f[D=\frac{1}{4}q^{2}+\frac{1}{27}p^{3}\f]
 * \f[U=\frac{1}{3}\arccos(\frac{3q}{2p})\sqrt{-\frac{3}{p}}\f]
 * \f[V=2\sqrt{-\frac{1}{3}p}\f]
 * \f[
 *  x=
 *   \begin{cases}
 *    0 & \text{if } p=0 \text{ and }q=0\\
 *    \frac{3q}{p}, -\frac{3q}{2p} & \text{if } D=0\\
 *    \sqrt[3]{-\frac{1}{2}+\sqrt{D}} + \sqrt[3]{-\frac{1}{2}-\sqrt{D}} & \text{if } D>0\\
 *    V\cdot \cos(U), V\cdot\cos(U-\frac{2}{3}\pi),V\cdot\cos(U-\frac{4}{3}\pi) & \text{if } D<0
 *   \end{cases}
 * \f]
 */
template<typename T>
hi_force_inline lean_vector<T> solveDepressedCubic(T const& p, T const& q) noexcept
{
    constexpr T oneForth = static_cast<T>(1) / static_cast<T>(4);
    constexpr T oneTwentySeventh = static_cast<T>(1) / static_cast<T>(27);

    if (p == 0.0 && q == 0.0) {
        return {static_cast<T>(0)};

    } else {
        hilet D = oneForth * q * q + oneTwentySeventh * p * p * p;

        if (D < 0 && p != 0.0) {
            // Has three real roots.
            return solveDepressedCubicTrig(p, q);

        } else if (D == 0 && p != 0.0) {
            // Has two real roots, or maybe one root
            hilet t0 = (static_cast<T>(3) * q) / p;
            hilet t1 = (static_cast<T>(-3) * q) / (static_cast<T>(2) * p);
            return {t0, t1, t1};

        } else {
            // Has one real root.
            return solveDepressedCubicCardano(p, q, D);
        }
    }
}

/*! Solve cubic function.
 * \f[ax^{3}+bx^{2}+cx+d=0\f]
 *
 * \f[p=\frac{3ac-b^{2}}{3a^{2}}\f],
 * \f[q=\frac{2b^{3}-9abc+27a^{2}d}{27a^{3}}\f]
 *
 * \f[x=\text{solveDepressedCube}(p,q)-\frac{b}{3a}\f]
 */
template<typename T>
hi_force_inline constexpr lean_vector<T> solvePolynomial(T const& a, T const& b, T const& c, T const& d) noexcept
{
    if (a == 0) {
        return solvePolynomial(b, c, d);

    } else {
        hilet p = (static_cast<T>(3) * a * c - b * b) / (static_cast<T>(3) * a * a);
        hilet q = (static_cast<T>(2) * b * b * b - static_cast<T>(9) * a * b * c + static_cast<T>(27) * a * a * d) /
            (static_cast<T>(27) * a * a * a);

        hilet b_3a = b / (static_cast<T>(3) * a);

        auto r = solveDepressedCubic(p, q);
        for (auto &x: r) {
            x -= b_3a;
        }
        
        return r;
    }
}

} // namespace hi::inline v1
