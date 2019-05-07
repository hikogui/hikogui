// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <complex>
#include <cmath>
#include <boost/assert.hpp>
#include <glm/glm.hpp>

namespace TTauri {

constexpr float pi = 3.14159265358979323846f;
constexpr float pi2_3 = 2.0f * pi / 3.0f;
constexpr float pi4_3 = 2.0f * pi2_3;
constexpr float oneThird = 1.0f / 3.0f;
constexpr float oneForth = 1.0f / 4.0f;
constexpr float oneTwentySeventh = 1.0f / 27.0f;


/*! Trigonometric solution for three real roots
 */
inline std::tuple<float,float,float> solveDepressedCubicTrig(float const p, float const q) {
    auto const U = oneThird * acosf(((3.0f*q) / (2.0f*p)) * sqrt(-3.0f/p));
    auto const V = 2.0f * sqrtf(-oneThird * p);

    auto const t0 = V * cosf(U);
    auto const t1 = V * cosf(U - pi2_3);
    auto const t2 = V * cosf(U - pi4_3);
    return { t0, t1, t2 };
}

inline float solveDepressedCubicCardano(float const p, float const q, float const D) {
    auto const sqrtD = sqrtf(D);
    auto const minusHalfQ = -0.5f * q;
    auto const v = cbrtf(minusHalfQ + sqrtD);
    auto const w = cbrtf(minusHalfQ - sqrtD);
    return v + w;
}

/*! Solve cubic function in the form.
 * t*t*t + p*t + q = 0
 */
inline std::tuple<float,float,float> solveDepressedCubic(float p, float q) {
    float t0, t1, t2;

    if (p == 0.0f && q == 0.0f) {
        t0 = t1 = t2 = 0.0f;

    } else {
        auto const D = oneForth*q*q + oneTwentySeventh*p*p*p;

        if (D < 0) {
            // Has three real roots.
            std::tie(t0, t1, t2) = solveDepressedCubicTrig(p, q);

        } else if (D == 0.0) {
            // Has two real roots, or maybe one root
            t0 = (3.0f*q) / p;
            t1 = t2 = (-3.0f*q) / (2.0f*p);

        } else {
            // Has one real root.
            t0 = t1 = t2 = solveDepressedCubicCardano(p, q, D);
        }
    }

    return { t0, t1, t2 };
}

/*! Solve cubic function.
 * a*x*x*x + b*x*x + c*x + d = 0
 */
inline std::tuple<float,float,float> solveCubic(float a, float b, float c, float d) {
    auto const p = (3.0f*a*c - b*b) / (3.0f*a*a);
    auto const q = (2.0f*b*b*b - 9.0f*a*b*c + 27.0f*a*a*d) / (27.0f*a*a*a);

    auto const [t0, t1, t2] = solveDepressedCubic(p, q);

    auto const b_3a = b / (3.0f*a);
    return {
        t0 - b_3a,
        t1 - b_3a,
        t2 - b_3a
    };
}

inline float viktorCross(glm::vec2 const a, glm::vec2 const b) {
    return a.x * b.y - a.y * b.x;
}

}
