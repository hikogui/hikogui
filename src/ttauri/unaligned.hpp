// Copyright Take Vos 2021
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include <cstdint>
#include <cstddef>

namespace tt {

/** Copy the byte from lsb to msb from src to dst.
 */
inline void unaligned_le_store(uint32_t src, std::byte *dst, size_t num_bytes) noexcept
{
    tt_axiom(num_bytes >= 1 && num_bytes <= 4);

    do {
        *(dst++) = static_cast<std::byte>(src);
        src >>= 8;
    } while (--num_bytes);
}


}

