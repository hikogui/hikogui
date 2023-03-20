// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

namespace hi::inline v1 {

enum class unicode_grapheme_cluster_break : uint8_t {
    other = 0,
    CR = 1,
    LF = 2,
    control = 3,
    extend = 4,
    ZWJ = 5,
    regional_indicator = 6,
    prepend = 7,
    spacing_mark = 8,
    L = 9,
    V = 10,
    T = 11,
    LV = 12,
    LVT = 13,
    extended_pictographic = 14
};

}
