// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>

namespace tt {

enum class unicode_bidi_bracket_type : uint8_t {
    n, ///< Not mirrored
    o, ///< Open bracket
    c, ///< Close bracket
    m ///< Mirror but not bracket
};

}