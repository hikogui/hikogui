// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/byte_string.hpp"
#include <nonstd/span>

namespace tt {

/** Inflate compressed data using the deflate algorithm
 * `bytes` should include at least 32 bit of trailer, for the overflow check which
 * will slightly overrun the actual compressed data for performance reasons.
 *
 * - gzip has a CRC32+ISIZE trailer.
 *   Since gzip has no end-of-segment indicator, we need to include the trailer
 *   in the byte array passed to inflate anyway.
 * - zlib has a 32 bit check value.
 *   Since zlib has no end-of-segment indicator, we need to include the trailer
 *   in the byte array passed to inflate anyway.
 * - png IDAT chunks include the full zlib-format, including the 32 bit check value.
 */
bstring inflate(nonstd::span<std::byte const> bytes, ssize_t &offset, ssize_t max_size=0x0100'0000);

}

