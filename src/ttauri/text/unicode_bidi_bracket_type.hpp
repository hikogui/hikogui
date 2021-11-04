// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

namespace tt::inline v1 {

enum class unicode_bidi_bracket_type : uint8_t {
    n, ///< Not mirrored
    o, ///< Open bracket
    c, ///< Close bracket
    m ///< Mirror but not bracket
};

}
