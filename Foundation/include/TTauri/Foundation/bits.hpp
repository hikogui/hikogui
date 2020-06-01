// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/exceptions.hpp"
#include <gsl/gsl>
#include <cstddef>

namespace TTauri {

/** Read a single bit of span of bytes
 * Bits are ordered LSB first.
 *
 * @param index The index of the bit in the byte span.
 */
[[nodiscard]] inline bool get_bit(gsl::span<std::byte const> buffer, ssize_t index)
{
    auto byte_index = index >> 3;
    auto bit_index = static_cast<uint8_t>(index & 7);

    parse_assert(byte_index < ssize(buffer));
    return static_cast<bool>(
        static_cast<int>(buffer[byte_index] >> bit_index) & 1
    );
} 

[[nodiscard]] inline bool get_bit_and_advance(gsl::span<std::byte const> buffer, ssize_t &index)
{
    return get_bit(buffer, index++);
}

/** Read a single bit of span of bytes
 * Bits are ordered LSB first.
 * Bits are copied as if the byte array is layed out from right to left, example:
 *
 *  7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    byte 1     |    byte 0     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *           :         :
 * index=6   +-+-+-+-+-+
 * length=5  | Return  |
 *           +-+-+-+-+-+
 *            4 3 2 1 0
 *
 * @param index The index of the bit in the byte span.
 * @param length the number of bits to return.
 */
[[nodiscard]] inline int get_bits(gsl::span<std::byte const> buffer, ssize_t index, int length)
{
    auto value = 0;

    auto todo = length;
    auto done = 0;
    while (todo) {
        auto byte_index = index >> 3;
        auto bit_index = static_cast<int>(index & 7);
        parse_assert(byte_index < ssize(buffer));

        auto available_bits = 8 - bit_index;
        auto nr_bits = available_bits < todo ? available_bits : todo;

        auto mask = (1 << nr_bits) - 1;

        auto tmp = static_cast<int>(buffer[byte_index] >> bit_index) & mask;
        value |= tmp << done;

        todo -= nr_bits;
        done += nr_bits;
        index += nr_bits;
    }

    return value;
} 

[[nodiscard]] inline int get_bits_and_advance(gsl::span<std::byte const> buffer, ssize_t &index, int length)
{
    int value = get_bits(buffer, index, length);
    index += length;
    return value;
}

}