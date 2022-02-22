// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

namespace tt::inline v1 {

enum class unicode_decomposition_type : uint8_t {
    canonical = 0, ///< Canonical decomposition.
    font = 1, ///< <font> Font variant (for example, a blackletter form).
    no_break = 2, ///< <no_break> No - break version of a space or hyphen.
    arabic = 3, ///< <initial> <medial> <final> <isolated> Arabic presentation forms.
    circle = 4, ///< <circle> Encircled form.
    math = 5, ///< <super> <sub> <fraction> Super-, sub-script and Vulgar-fraction forms
    asian = 6, ///< <vertical> <wide> <narrow> <small> <square> asian compatibility forms.
    compat = 7 ///< <compat> Otherwise unspecified compatibility character
};

}
