// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file numbers.hpp
 *
 * This file contains constants and conversion functions.
 */

#pragma once

#include "concepts.hpp"
#include <concepts>

namespace hi::inline v1 {

/** A large number.
* 
* This number 16777215 (2^24 - 1). Integers beyond this number may not be accurately represented by a float. 
*/
template<typename T> requires (std::numeric_limits<T>::max() >= 16777215)
constexpr auto large_number_v = T{16777215};

/** The number of points (typography) per inch.
 */
template<std::floating_point T>
constexpr auto points_per_inch_v = T{72.0};

/** The number of device independent pixels per inch.
 *
 * device independent pixels per platform:
 *  - win32: 96 dp/inch
 *  - MacOS: 72 dp/inch
 *  - Android: 160 dp/inch (80 double dp/inch)
 *
 * The Android dp size is almost halfway between win32 and MacOS, which we will use for hikogui.
 */
template<std::floating_point T>
constexpr auto dp_per_inch_v = T{80.0};

template<std::floating_point T>
constexpr auto points_to_dp_scale_v = dp_per_inch_v<T> / points_per_inch_v<T>;

/** Convert points to device independent pixels.
 */
template<std::floating_point T>
constexpr T points_to_dp(T x) noexcept
{
    return x * points_to_dp_scale_v<T>;
}

/** @see points_per_inch_v
 */
constexpr double points_per_inch = points_per_inch_v<double>;

/** @see dp_per_inch_v
 */
constexpr double dp_per_inch = dp_per_inch_v<double>;

} // namespace hi::inline v1
