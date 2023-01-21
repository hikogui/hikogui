// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../byte_string.hpp"
#include <span>

namespace hi::inline v1 {

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
bstring inflate(std::span<std::byte const> bytes, std::size_t &offset, std::size_t max_size = 0x0100'0000);

} // namespace hi::inline v1
