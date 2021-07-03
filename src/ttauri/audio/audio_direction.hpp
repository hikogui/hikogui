// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt {

enum class audio_direction : unsigned char {
    none = 0b00,
    input = 0b01,
    output = 0b10,
    bidirectional = 0b11
};


}
