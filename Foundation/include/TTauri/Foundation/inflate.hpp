// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/byte_string.hpp"
#include <nonstd/span>

namespace TTauri {

/** Inflate compressed data using the deflate algorithm
 * `bytes` should include at least 64 bit of trailer, for the overflow check which
 * will slightly overrun the actual compressed data for performance reasons.
 *
 * - gzip has a CRC32+ISIZE trailer.
 *   This is not a problem because gzip does not have a segment-length indicator,
 *   so we must inlude the whole file in bytes.
 * - png has a CRC32 trailer.
 *   This is not enough, but there will always be another chunk after the IDAT chuck
 *   of which 32 bits may be borrowed.
 * - zlib only CRC32 trailer is included.
 *   This is not enough, the library that handles zlib data should append 32 bits of data
 *   to handle the overrun.
 */
bstring inflate(nonstd::span<std::byte const> bytes, ssize_t &offset, ssize_t max_size=0x0100'0000);

}

