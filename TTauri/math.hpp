// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <complex>

namespace TTauri {

/*! Trigonometric solution for three real roots
 * for k=0, i.e. we can remove (2*pi*k)/3 term.
 */
inline double solveDepressedCubicTrigK0(double const p, double const q) {
    assert(p < 0.0);

    auto const u = ((3.0*q) / (2.0*p)) * sqrt(-3.0/p);
    assert(u >= -1.0 && u <= 1.0);

    return 2.0*sqrt(p/-3.0) * cos(1/3.0 * arccos(u));
}

inline std::tuple<double,double,double> solveDepressedCubic(double p, double q) {
    auto const D = q*q/4.0 + p*p*p/27.0;

    if (D < 0) {
        // Has three real roots.
        auto const t0 = solveDepressedCubicTrigK0(p, q);
        auto const t2 = -solveDepressedCubicTrigK0(p, -q);
        auto const t1 = t0 - t2;
        return { t0, t1, t2 };

    } else (D == 0.0) {
        // Has two real roots, or maybe one root
        auto const t0 = (3.0*q) / p;
        auto const t12 = (-3.0*q) / (2.0*p);
        return { t0, t12, t12 };

    } else {
        // Has one real root.
        auto const sqrtD = sqrt(D);
        auto const q_m2 = q / -2.0;
        auto const v = cbrt(q_m2 + sqrtD);
        auto const w = cbrt(q_m2 - sqrtD);
        auto const t012 = v + w;
        return { t012, t012, t012 };
    }
}


/*! Solve cubic function.
 * a*x*x*x + b*x*x + c*x + d = 0
 */
inline std::tuple<double,double,double> solveCubic(double a, double b, double c, double d) {
    auto const p = (3*a*c - b*b) / (3*a*a);
    auto const q = (2*b*b*b - 9*a*b*c + 27*a*a*d) / (27*a*a*a);

    auto const [t1, t2, t2] = solveDepressedCubic(p, q);

    auto const b_3a = b / (3*a);
    return {
        t1 - b_3a,
        t2 - b_3a,
        t3 - b_3a
    }
}

}
