// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/required.hpp"

#include <complex>
#include <cmath>
#include <limits>
#include <boost/assert.hpp>
#include <glm/glm.hpp>
#include <gsl/gsl>

namespace TTauri {

constexpr float pi = 3.14159265358979323846f;
constexpr float pi2_3 = 2.0f * pi / 3.0f;
constexpr float pi4_3 = 2.0f * pi2_3;
constexpr float oneThird = 1.0f / 3.0f;
constexpr float oneForth = 1.0f / 4.0f;
constexpr float oneTwentySeventh = 1.0f / 27.0f;

template<typename T, int N>
struct results {
    static constexpr size_t maxCount = N;
    using element_type = T;
    using const_iterator = typename std::array<T,N>::const_iterator;

    ptrdiff_t count;
    std::array<T,N> value;

    results() : count(0), value() {}

    results(T a) : count(1), value() {
        value[0] = a;
    }

    results(T a, T b) : count(2), value() {
        value[0] = a;
        value[1] = b;
        sort();
    }

    results(T a, T b, T c) : count(3), value() {
        value[0] = a;
        value[1] = b;
        value[2] = c;
        sort();
    }

    template<int O, typename std::enable_if_t<std::less<int>{}(O,N), int> = 0>
    results(results<T,O> const &other) : count(other.count), value() {
        if constexpr (other.maxCount > 0) {
            for (size_t i = 0; i < other.maxCount; i++) {
                value[i] = other.value[i];
            }
        }
    }

    size_t size() const {
        return (count >= 0) ? count : 0;
    }

    bool hasInfiniteResults() const {
        return count < 0;
    }

    const_iterator begin() const {
        return value.begin();
    }

    const_iterator end() const {
        return value.begin() + size();
    }

    void sort() {
        std::sort(value.begin(), value.begin() + size());
    }

    void add(T a) {
        value.at(count++) = a;
        sort();
    }

    results operator-(T a) const {
        results r = *this;
        for (size_t i = 0; i < maxCount; i++) {
            r.value[i] -= a;
        }
        return r;
    }

    T maxAbsDiff(results const &other) const {
        if (this->count != other.count) {
            return std::numeric_limits<T>::infinity();
        }

        T maxDiff = 0.0;
        assert(this->count <= maxCount);
        for (ptrdiff_t i = 0; i < this->count; i++) {
            let diff = std::abs(this->value[i] - other.value[i]);
            maxDiff = std::max(maxDiff, diff);
        }
        return maxDiff;
    }
};

template<typename T>
inline results<T,0> infinitResults() {
    results<T,0> r;
    r.count = -1;
    return r;
}

template<typename T, int N>
inline std::ostream& operator<<(std::ostream& os, const results<T,N> &r)
{
    os << "[";
    assert(r.count <= r.maxCount);
    for (ptrdiff_t i = 0; i < r.count; i++) {
        if (i > 0) {
            os << ", ";
        }
        os << r.value[i];
    }
    os << "]";
    return os;
}

using results1 = results<float,1>;
using results2 = results<float,2>;
using results3 = results<float,3>;

/*! Solve line function.
 * a*x + b = 0;
 */
inline results1 solveLinear(float const a, float const b) {
    if (a != 0.0) {
        return { -(b / a) };
    } else if (b == 0.0) {
        // Any value of x is correct.
        return infinitResults<float>();
    } else {
        // None of the values for x is correct.
        return {};
    }
}

/*! Solve quadratic function.
 * ax*x + bx + c = 0
 */
inline results2 solveQuadratic(float const a, float const b, float const c) {
    if (a == 0) {
        return solveLinear(b, c);
    } else {
        let D = b*b - 4.0f*a*c;
        if (D < 0.0) {
            return {};
        } else if (D == 0) {
            return { -b / (2.0f*a) };
        } else {
            let Dsqrt = sqrtf(D);
            return {
                (-b - Dsqrt) / (2.0f*a),
                (-b + Dsqrt) / (2.0f*a)
            };
        }
    }
}

/*! Trigonometric solution for three real roots
 */
inline results3 solveDepressedCubicTrig(float const p, float const q) {
    let U = oneThird * acosf(((3.0f*q) / (2.0f*p)) * sqrt(-3.0f/p));
    let V = 2.0f * sqrtf(-oneThird * p);

    let t0 = V * cosf(U);
    let t1 = V * cosf(U - pi2_3);
    let t2 = V * cosf(U - pi4_3);
    return { t0, t1, t2 };
}

inline results3 solveDepressedCubicCardano(float const p, float const q, float const D) {
    let sqrtD = sqrtf(D);
    let minusHalfQ = -0.5f * q;
    let v = cbrtf(minusHalfQ + sqrtD);
    let w = cbrtf(minusHalfQ - sqrtD);
    return { v + w };
}

/*! Solve cubic function in the form.
 * t*t*t + p*t + q = 0
 */
inline results3 solveDepressedCubic(float p, float q) {
    if (p != 0.0f || q != 0.0f) {
        let D = oneForth*q*q + oneTwentySeventh*p*p*p;

        if (D < 0) {
            // Has three real roots.
            return solveDepressedCubicTrig(p, q);

        } else if (D == 0.0) {
            // Has two real roots, or maybe one root
            let t0 = (3.0f*q) / p;
            let t1 = (-3.0f*q) / (2.0f*p);
            return {t0, t1};

        } else {
            // Has one real root.
            return solveDepressedCubicCardano(p, q, D);
        }
    } else {
        return {0.0};
    }
}

/*! Solve cubic function.
 * a*x*x*x + b*x*x + c*x + d = 0
 */
inline results3 solveCubic(float a, float b, float c, float d) {
    if (a == 0.0f) {
        return solveQuadratic(b, c, d);

    } else {
        let p = (3.0f*a*c - b*b) / (3.0f*a*a);
        let q = (2.0f*b*b*b - 9.0f*a*b*c + 27.0f*a*a*d) / (27.0f*a*a*a);

        let r = solveDepressedCubic(p, q);

        let b_3a = b / (3.0f*a);

        return r - b_3a;
    }
}


}
