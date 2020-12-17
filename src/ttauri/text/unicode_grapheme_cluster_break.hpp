// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>

namespace tt {

enum class unicode_grapheme_cluster_break : uint8_t {
    Other,
    CR,
    LF,
    Control,
    Extend,
    ZWJ,
    Regional_Indicator,
    Prepend,
    SpacingMark,
    L,
    V,
    T,
    LV,
    LVT,
    Extended_Pictographic
};

}