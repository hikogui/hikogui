// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "math.hpp"
#include <numbers>
#include <array>

namespace tt {

template<typename T, int N>
struct results {
    static constexpr int maxCount = N;
    using element_type = T;
    using array_type = std::array<T,N>;
    using const_iterator = typename array_type::const_iterator;

    int count;
    array_type value;

    results() noexcept : count(0), value() {}

    results(T a) noexcept : count(1), value() {
        value[0] = a;
    }

    results(T a, T b) noexcept : count(2), value() {
        value[0] = a;
        value[1] = b;
        sort();
    }

    results(T a, T b, T c) noexcept : count(3), value() {
        value[0] = a;
        value[1] = b;
        value[2] = c;
        sort();
    }

    template<int O, typename std::enable_if_t<O < N, int> = 0>
    results(results<T,O> const &other) noexcept : count(other.count), value() {
        if constexpr (O > 0) {
            for (int i = 0; i < other.maxCount; i++) {
                value[i] = other.value[i];
            }
        }
    }

    int size() const noexcept {
        return (count >= 0) ? count : 0;
    }

    bool hasInfiniteResults() const noexcept {
        return count < 0;
    }

    const_iterator begin() const noexcept {
        return value.begin();
    }

    const_iterator end() const noexcept {
        return value.begin() + size();
    }

    void sort() noexcept {
        std::sort(value.begin(), value.begin() + size());
    }

    void add(T a) noexcept {
        value.at(count++) = a;
        sort();
    }
};

template<typename T, int N, typename U>
inline results<T, N> operator-(results<T, N> lhs, U const &rhs) noexcept
{
    for (int i = 0; i < lhs.maxCount; i++) {
        lhs.value[i] -= rhs;
    }
    return lhs;
}

template<typename T>
inline results<T,0> infinitResults() noexcept {
    results<T,0> r;
    r.count = -1;
    return r;
}

template<typename T, int N>
inline std::ostream& operator<<(std::ostream& os, results<T,N> const &r)
{
    os << "[";
    tt_assert(r.count <= r.maxCount);
    for (int i = 0; i < r.count; i++) {
        if (i > 0) {
            os << ", ";
        }
        os << r.value.at(i);
    }
    os << "]";
    return os;
}

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
inline results<T,1> solvePolynomial(T const &a, T const &b) noexcept {
    if (a != 0) {
        return { -(b / a) };
    } else if (b == 0) {
        // Any value of x is correct.
        return infinitResults<T>();
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
inline results<T,2> solvePolynomial(T const &a, T const &b, T const &c) noexcept {
    if (a == 0) {
        return solvePolynomial(b, c);
    } else {
        ttlet D = b*b - static_cast<T>(4)*a*c;
        if (D < 0) {
            return {};
        } else if (D == 0) {
            return { -b / (static_cast<T>(2)*a) };
        } else {
            ttlet Dsqrt = sqrt(D);
            return {
                (-b - Dsqrt) / (static_cast<T>(2)*a),
                (-b + Dsqrt) / (static_cast<T>(2)*a)
            };
        }
    }
}

/*! Trigonometric solution for three real roots
*/
template<typename T>
inline results<T,3> solveDepressedCubicTrig(T const &p, T const &q) noexcept {
    constexpr T oneThird = static_cast<T>(1) / static_cast<T>(3);
    constexpr T pi2_3 = (static_cast<T>(2) / static_cast<T>(3)) * std::numbers::pi_v<T>;
    constexpr T pi4_3 = (static_cast<T>(4) / static_cast<T>(3)) * std::numbers::pi_v<T>;

    ttlet U = oneThird * acos(((static_cast<T>(3)*q) / (static_cast<T>(2)*p)) * sqrt(static_cast<T>(-3)/p));
    ttlet V = static_cast<T>(2) * sqrt(-oneThird * p);

    ttlet t0 = V * cos(U);
    ttlet t1 = V * cos(U - pi2_3);
    ttlet t2 = V * cos(U - pi4_3);
    return { t0, t1, t2 };
}

template<typename T>
inline results<T,3> solveDepressedCubicCardano(T const &p, T const &q, T const &D) noexcept {
    ttlet sqrtD = sqrt(D);
    ttlet minusHalfQ = static_cast<T>(-0.5) * q;
    ttlet v = cbrt(minusHalfQ + sqrtD);
    ttlet w = cbrt(minusHalfQ - sqrtD);
    return { v + w };
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
inline results<T,3> solveDepressedCubic(T const &p, T const &q) noexcept {
    constexpr T oneForth = static_cast<T>(1) / static_cast<T>(4);
    constexpr T oneTwentySeventh = static_cast<T>(1) / static_cast<T>(27);

    if (p == 0.0 && q == 0.0) {
        return { static_cast<T>(0) };

    } else {
        ttlet D = oneForth*q*q + oneTwentySeventh*p*p*p;

        if (D < 0 && p != 0.0) {
            // Has three real roots.
            return solveDepressedCubicTrig(p, q);

        } else if (D == 0 && p != 0.0) {
            // Has two real roots, or maybe one root
            ttlet t0 = (static_cast<T>(3)*q) / p;
            ttlet t1 = (static_cast<T>(-3)*q) / (static_cast<T>(2)*p);
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
inline results<T,3> solvePolynomial(T const &a, T const &b, T const &c, T const &d) noexcept {
    if (a == 0) {
        return solvePolynomial(b, c, d);

    } else {
        ttlet p = (static_cast<T>(3)*a*c - b*b) / (static_cast<T>(3)*a*a);
        ttlet q = (static_cast<T>(2)*b*b*b - static_cast<T>(9)*a*b*c + static_cast<T>(27)*a*a*d) / (static_cast<T>(27)*a*a*a);

        ttlet r = solveDepressedCubic(p, q);

        ttlet b_3a = b / (static_cast<T>(3)*a);

        return r - b_3a;
    }
}

}
