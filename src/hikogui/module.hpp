// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "image/module.hpp"
#include "color/module.hpp"
#include "geometry/module.hpp"
#include "SIMD/module.hpp"
#include "char_maps/module.hpp"
#include "font/module.hpp"
#include "concurrency/module.hpp"
#include "utility/module.hpp"

/** @file module.hpp
* 
* 
* Dependencies:
*  - utility: -
*  - SIMD: utility
*  - geometry: SIMD, utility, concurrency
*  - color: geometry, SIMD, utility
*  - image: geometry, SIMD, color, utility
*  - char_maps: utility
* 
*/

/** @namespace hi The HikoGUI namespace.
 */
namespace hi {

/** @namespace hi::v1 The HikoGUI API version 1.
 */
inline namespace v1 {

/** @namespace hi::v1::geo HikoGUI geometry types.
 */
namespace geo{}

} // namespace v1
} // namespace hi

/** @namespace v1 DOXYGEN BUG
 * @brief Doxygen can't handle `namespace hi::inline v1 {}` syntax.
 * All files should be changed to use old-style inline namespace syntax.
 */
namespace v1{}
