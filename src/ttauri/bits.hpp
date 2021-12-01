// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "exception.hpp"
#include "assert.hpp"
#include <span>
#include <cstddef>

namespace tt::inline v1 {

/** Read a single bit from span of bytes
 * Bits are ordered LSB first.
 *
 * @param buffer The buffer of bytes to extract the bit from.
 * @param index The index of the bit in the byte span.
 */
[[nodiscard]] inline bool get_bit(std::span<std::byte const> buffer, size_t &index) noexcept
{
    auto byte_index = index >> 3;
    auto bit_index = index & 7;
    ++index;

    tt_axiom(byte_index < buffer.size());
    return static_cast<bool>(static_cast<uint8_t>(buffer[byte_index] >> bit_index) & 1);
}

/** Read a bits from of span of bytes
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
 * @param buffer The buffer of bytes to extract bits from.
 * @param index The index of the bit in the byte span.
 * @param length the number of bits to return.
 */
[[nodiscard]] inline size_t get_bits(std::span<std::byte const> buffer, size_t &index, size_t length) noexcept
{
    tt_axiom(length <= sizeof(size_t) * CHAR_BIT);

    auto value = 0;

    auto todo = length;
    auto done = 0_uz;
    while (todo) {
        auto byte_index = index >> 3;
        auto bit_index = index & 7;
        tt_axiom(byte_index < buffer.size());

        auto available_bits = 8 - bit_index;
        auto nr_bits = std::min(available_bits, todo);

        auto mask = (1 << nr_bits) - 1;

        auto tmp = static_cast<int>(buffer[byte_index] >> bit_index) & mask;
        value |= tmp << done;

        todo -= nr_bits;
        done += nr_bits;
        index += nr_bits;
    }

    return value;
}

} // namespace tt::inline v1
