// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

namespace hi::inline v1 {

// Windows.h defines small as a macro.
#ifdef small
#undef small
#endif

enum class unicode_decomposition_type : uint8_t {
    none, // No decomposition.
    canonical, ///< Canonical decomposition.
    font, ///< \<font\> Font variant (for example, a blackletter form).
    noBreak, ///< \<noBreak\> No - break version of a space or hyphen.
    initial, ///< \<initial\> Arabic presentation forms.
    medial, ///< \<medial\> Arabic presentation forms.
    _final, ///< \<final\> Arabic presentation forms.
    isolated, ///< \<isolated\> Arabic presentation forms.
    circle, ///< \<circle\> Encircled form.
    super, ///< \<super\> Super-, sub-script and Vulgar-fraction forms
    sub, ///< \<sub\> Super-, sub-script and Vulgar-fraction forms
    fraction, ///< \<fraction\> Super-, sub-script and Vulgar-fraction forms
    vertical, ///< \<vertical\> asian compatibility forms.
    wide, ///< \<wide\> asian compatibility forms.
    narrow, ///< \<narrow\> asian compatibility forms.
    small, ///< \<small\>asian compatibility forms.
    square, ///< \<square\> asian compatibility forms.
    compat ///< \<compat\> Otherwise unspecified compatibility character
};

}
