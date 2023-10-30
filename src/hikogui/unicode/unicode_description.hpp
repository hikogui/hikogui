// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.unicode.unicode_description);


hi_export namespace hi::inline v1 {

constexpr char32_t unicode_replacement_character = U'\ufffd';
constexpr char32_t unicode_LF = U'\n';
constexpr char32_t unicode_VT = U'\v';
constexpr char32_t unicode_FF = U'\f';
constexpr char32_t unicode_CR = U'\r';
constexpr char32_t unicode_NEL = U'\u0085';
constexpr char32_t unicode_LS = U'\u2028';
constexpr char32_t unicode_PS = U'\u2029';

} // namespace hi::inline v1
