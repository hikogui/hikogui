

#pragma once

#include "fixed_string.hpp"
#include <cstdint>

namespace tt {

/** A counter generated of some clock.
 */
class clock_counter {
public:
    uint64_t count;
    uint32_t id;
};

}

