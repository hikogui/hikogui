//---------------------------------------------------------------------------
/// \file   approx.hpp
/// \brief  See if two values are equal within a margin
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cmath>

namespace rhea
{

/** Return true iff a and b are approximately the same. */
inline bool approx(double a, double b)
{
    const double epsilon = 1.0e-8;
    if (a > b) {
        return (a - b) < epsilon;
    } else {
        return (b - a) < epsilon;
    }
}

/** Return true iff a is almost zero. */
inline bool near_zero(double a)
{
    return approx(a, 0.0);
}

} // namespace rhea
