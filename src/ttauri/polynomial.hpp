// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/math.hpp"

namespace tt {

template<typename T, int N>
struct results {
    static constexpr int maxCount = N;
    using element_type = T;
    using const_iterator = typename std::array<T,N>::const_iterator;

    int count;
    std::array<T,N> value;

    results() noexcept : count(0), value() {}

    gsl_suppress(bounds.4)
    results(T a) noexcept : count(1), value() {
        value[0] = a;
    }

    gsl_suppress(bounds.4)
    results(T a, T b) noexcept : count(2), value() {
        value[0] = a;
        value[1] = b;
        sort();
    }

    gsl_suppress(bounds.4)
    results(T a, T b, T c) noexcept : count(3), value() {
        value[0] = a;
        value[1] = b;
        value[2] = c;
        sort();
    }

    template<int O, typename std::enable_if_t<std::less<int>{}(O,N), int> = 0>
    gsl_suppress2(bounds.2,bounds.4)
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
gsl_suppress2(bounds.2,bounds.4)
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
* x=
* \begin{cases}
*  \frac{-b}{a}, & \text{if } a\ne0\\
*  \in\mathbb{R}, & \text{if } a=0,b=0\\
*  \o & \text{if } a=0,b\ne0
* \end{cases}
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
* ax^{2}+bx+c=0
*
* D=b^{2}-4ac\\
* x=
* \begin{cases}
* \frac{-b}{2a}, & \text{if } D=0\\
* \frac{-b -\sqrt{D}}{2a}, \frac{-b +\sqrt{D}}{2a} & \text{if } D>0\\
* \o & \text{if } D<0
* \end{cases}
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
    constexpr T pi2_3 = (static_cast<T>(2) / static_cast<T>(3)) * static_cast<T>(pi);
    constexpr T pi4_3 = (static_cast<T>(4) / static_cast<T>(3)) * static_cast<T>(pi);

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
* t^{3}+pt+q=0
*
* D=\frac{1}{4}q^{2}+\frac{1}{27}p^{3}\\
* U=\frac{1}{3}\arccos(\frac{3q}{2p})\sqrt{-\frac{3}{p}}\\
* V=2\sqrt{-\frac{1}{3}p}\\
* x=
* \begin{cases}
* 0 & \text{if } p=0 \text{ and }q=0\\
* \frac{3q}{p}, -\frac{3q}{2p} & \text{if } D=0\\
* \sqrt[3]{-\frac{1}{2}+\sqrt{D}} + \sqrt[3]{-\frac{1}{2}-\sqrt{D}} & \text{if } D>0\\
* V\cdot \cos(U), V\cdot\cos(U-\frac{2}{3}\pi),V\cdot\cos(U-\frac{4}{3}\pi) & \text{if } D<0
* \end{cases}
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
* ax^{3}+bx^{2}+cx+d=0
*
* p=\frac{3ac-b^{2}}{3a^{2}},
* q=\frac{2b^{3}-9abc+27a^{2}d}{27a^{3}}\\
* \\
* x=\text{solveDepressedCube}(p,q)-\frac{b}{3a}
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
