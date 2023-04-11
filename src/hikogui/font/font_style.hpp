// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file font/font_style.hpp Defines the `font_style` type.
 */

#pragma once

namespace hi {
inline namespace v1 {

/** The different styles a font-family comes with.
 *
 * Either `normal`, `italic` or `oblique`. Although technically there is a difference
 * between `italic` and `oblique` this difference is small and font-families rarely include
 * both those styles; HikoGUI will treat `italic` and `oblique` as the same.
 *
 * @ingroup font
 */
enum class font_style {
    /** A font that is normal, non-italic.
     */
    normal = 0,

    /** A font that is italic.
     */
    italic = 1,

    /** A font that is oblique.
     */
    oblique = 1
};

}}
