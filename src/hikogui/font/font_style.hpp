// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file font/font_style.hpp Defines the `font_style` type.
 */

#pragma once

#include "../utility/utility.hpp"
#include "../coroutine/coroutine.hpp"
#include <string_view>
#include <coroutine>

hi_export_module(hikogui.font.font_style);

hi_export namespace hi {
inline namespace v1 {

/** The different styles a font-family comes with.
 *
 * Either `normal`, `italic` or `oblique`. Although technically there is a difference
 * between `italic` and `oblique` this difference is small and font-families rarely include
 * both those styles; HikoGUI will treat `italic` and `oblique` as the same.
 *
 * @ingroup font
 */
hi_export enum class font_style {
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

// clang-format off
constexpr auto font_style_metadata = enum_metadata{
    font_style::normal, "normal",
    font_style::italic, "italic",
};
// clang-format on

hi_export [[nodiscard]] hi_inline generator<font_style> alternatives(font_style start) noexcept
{
    if (start == font_style::normal) {
        co_yield font_style::normal;
        co_yield font_style::italic;
    } else {
        co_yield font_style::italic;
        co_yield font_style::normal;
    }
}

}}
