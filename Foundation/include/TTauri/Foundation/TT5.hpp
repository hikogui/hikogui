// Copyright 2020 Pokitec
// All rights reserved.

/** @file TT5.hpp
 * @brief TT5 A 5 bit code designed for encoding identifiers.
 * 
 *  | #  | P0  | P1  | P2  |
 *  | --:|:--- |:--- |:--- |
 *  | 00 | NUL | NUL | NUL |
 *  | 01 | a   | A   | 0   |
 *  | 02 | b   | B   | 1   |
 *  | 03 | c   | C   | 2   |
 *  | 04 | d   | D   | 3   |
 *  | 05 | e   | E   | 4   |
 *  | 06 | f   | F   | 5   |
 *  | 07 | g   | G   | 6   |
 *  | 08 | h   | H   | 7   |
 *  | 09 | i   | I   | 8   |
 *  | 0a | j   | J   | 9   |
 *  | 0b | k   | K   | ,   |
 *  | 0c | l   | L   | :   |
 *  | 0d | m   | M   | ;   |
 *  | 0e | n   | M   | /   |
 *  | 0f | o   | O   | LF  |
 *  | 10 | p   | P   | B0  |
 *  | 11 | q   | Q   | B1  |
 *  | 12 | r   | R   | B2  |
 *  | 13 | s   | S   | B3  |
 *  | 14 | t   | T   | B4  |
 *  | 15 | u   | U   | B5  |
 *  | 16 | v   | V   | B6  |
 *  | 17 | w   | W   | B7  |
 *  | 18 | x   | X   | L0  |
 *  | 19 | y   | Y   | L1  |
 *  | 1a | z   | Z   | L2  |
 *  | 1b | _   | _   | _   |
 *  | 1c | .   | .   | .   |
 *  | 1d | -   | -   | -   |
 *  | 1e | S1  | S0  | S0  |
 *  | 1f | S2  | S2  | S1  |
 * 
 * # Pages
 * There are three pages.
 * The current- and locked page is page 0 at the start of the text.
 * 
 * By using the commands `S0`, `S1` or `S2` you can temporarilly switch
 * the current page until a single character is emited, afterwards the
 * current page is switched back to the locked page.
 * 
 * By using the command `L0`, `L1` or `L2` you can change the current- and
 * locked page.
 * 
 * # Binary
 * The page 2 commands `B*` are used to emit a single byte.
 * The lower 3 bits of the `B*` command are used as the high
 * 3-bits of the byte and the next 5 bits are used for the lower
 * 5 bits of the byte.
 * 
 * # End of text
 * End of text is denoted by the NUL character or when there is no more
 * room in the integer that contains the text.
 */

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/throw_exception.hpp"
#include "TTauri/Foundation/bigint.hpp"
#include <string>
#include <string_view>
#include <cstdint>

namespace TTauri {

/** Create a tt5_code.
* @return 11:10 page_nr, 9:5 prefix-code, 4:0 data (pre-shifted by 48 bits)
*/
[[nodiscard]] constexpr uint16_t tt5_code(uint16_t page, uint16_t prefix, uint16_t value)
{
    return (page << 10) | (prefix << 5) | value;
}

/** Convert a unicode character to a tt5-code.
* @return 11:10 page_nr, 9:5 prefix-code, 4:0 data (pre-shifted by 48 bits)
*/
[[nodiscard]] constexpr uint16_t tt5_code_table_generate_entry(uint8_t c) noexcept
{
    switch (c) {
    case '\0': return tt5_code(0, 0, 0);
    case '_': return tt5_code(3, 0, 0x1b);
    case '.': return tt5_code(3, 0, 0x1c);
    case '-': return tt5_code(3, 0, 0x1d);
    case ',': return tt5_code(2, 0, 0x0b);
    case ':': return tt5_code(2, 0, 0x0c);
    case ';': return tt5_code(2, 0, 0x0d);
    case '/': return tt5_code(2, 0, 0x0e);
    case '\n': return tt5_code(2, 0, 0x0f);
    default:
        if (c >= 'a' && c <= 'z') {
            return tt5_code(0, 0, (c - 'a') + 1);
        } else if (c >= 'A' && c <= 'Z') {
            return tt5_code(1, 0, (c - 'A') + 1);
        } else if (c >= '0' && c <= '9') {
            return tt5_code(2, 0, (c - '0') + 1);
        } else {
            auto c_ = static_cast<uint16_t>(c);
            return tt5_code(2, 0x10 | (c_ >> 5), (c_ & 0x1f)); // A0-A7
        }
    }
}

constexpr auto tt5_code_table_generator() noexcept
{
    std::array<uint16_t,256> r{};

    for (ssize_t i = 0; i != ssize(r); ++i) {
        r[i] = tt5_code_table_generate_entry(static_cast<uint8_t>(i));
    }

    return r;
}

constexpr auto tt5_code_table = tt5_code_table_generator();

constexpr uint64_t tt5_code_from_char(uint8_t c) noexcept
{
    return static_cast<uint64_t>(tt5_code_table[c]) << 48;
}

/*
* @param code 11:10 page_nr, 9:5 prefix-code, 4:0 data
*/
template<typename T>
constexpr ssize_t tt5_add_code(T &r, uint16_t code) noexcept
{
    // Strip off the page_nr
    code &= 0x3ff;

    ssize_t nr_bits = static_cast<bool>(code >> 5) ? 10 : 5;

    // If there is a prefix, then we need to add two code-units.
    r <<= static_cast<unsigned int>(nr_bits);

    // If prefix is zero, it doesn't matter if we 'or' the prefix on top
    // of the non-shifted part of r.
    r |= code;

    return nr_bits;
}

/** Check if we want to use the lock command.
 * We need at least 4 consequtive characters in the same page.
 * We ignore page 3 (character '.', '-', '_').
 */
[[nodiscard]] constexpr bool tt5_want_to_lock(uint64_t ring) noexcept
{
    ring >>= 10;
    let next_page = ring & 3;

    bool r = true;
    for (int i = 0; i != 3; ++i) {
        ring >>= 16;
        let later_page = ring & 3;
        r = r && (later_page == 3 || later_page == next_page);
    }
    return r;
}

/** Encode a UTF-8 string into an integer using TT5-encoding.
 * @param str Nul terminated UTF-8 string.
 * @return The string packed into an integer, with the last
 *         character in the least significant bits.
 */
template<typename T>
[[nodiscard]] constexpr T tt5_encode(char const *str)
{
    auto r = T{};
    ssize_t nr_bits = 0;
    constexpr ssize_t max_nr_bits = sizeof (T) * CHAR_BIT;
    uint64_t locked_page = 0;

    // Start with adding 4 codes into the ring buffer.
    uint64_t ring = 0;
    uint64_t later_code = 1; // Not end-of-text.
    for (int i = 0; i != 4; ++i) {
        later_code = (later_code) ? tt5_code_from_char(*(str++)) : 0;
        ring >>= 16;
        ring |= later_code;
    }

    while (ring) {
        let next_page = (ring >> 10) & 3;

        if (next_page != 3 && next_page != locked_page) {
            if (tt5_want_to_lock(ring)) {
                if (locked_page != 2) {
                    r <<= 5;
                    r |= 0x1f; // S2
                    _parse_assert2((nr_bits += 5) <= max_nr_bits, "String too long");
                }

                r <<= 5;
                r |= 0x18 + next_page; // L0, L1, L2
                _parse_assert2((nr_bits += 5) <= max_nr_bits, "String too long");

                locked_page = next_page;

            } else {
                bool second_shift = (locked_page == 0 && next_page == 2) || (locked_page != 0 && next_page != 0);

                r <<= 5;
                r |= 0x1e + static_cast<int>(second_shift); // S0, S1, S2
                _parse_assert2((nr_bits += 5) <= max_nr_bits, "String too long");
            }
        }

        nr_bits += tt5_add_code(r, static_cast<uint16_t>(ring));
        _parse_assert2(nr_bits <= max_nr_bits, "String too long");

        // Add one code to the ring buffer.
        later_code = (later_code) ? tt5_code_from_char(*(str++)) : 0;
        ring >>= 16;
        ring |= later_code;
    }

    return r;
}

/** Encode a UTF-8 string into an integer using TT5-encoding.
* @param str UTF-8 string.
* @return The string packed into an integer, with the last
*         character in the least significant bits.
*/
template<typename T>
[[nodiscard]] constexpr T tt5_encode(std::string const &s)
{
    return tt5_encode<T>(s.data());
}

/** Reverse the character in a TT5-packet-integer
* @param value The string packed into an integer, with the last
*         character in the least significant bits.
* @return The string packed into an integer, with the first
*         character in the least significant bits.
*/
template<typename T>
constexpr T tt5_reverse(T value) noexcept
{
    auto r = T{};

    while (value) {
        r <<=5;
        r |= value & 0x1f;
        value >>= 5;
    }

    return r;
}

[[nodiscard]] constexpr uint8_t char_from_tt5_page0(char *&str, uint8_t code, uint8_t locked_page) noexcept
{
    switch (code) {
    case 0x00: *(str++) = 0; return locked_page;
    case 0x1b: *(str++) = '_'; return locked_page;
    case 0x1c: *(str++) = '.'; return locked_page;
    case 0x1d: *(str++) = '-'; return locked_page;
    case 0x1e: return 1; // S1
    case 0x1f: return 2; // S2
    default:
        *(str++) = (static_cast<char>(code) - 1) + 'a';
        return locked_page;
    }
}

[[nodiscard]] constexpr uint8_t char_from_tt5_page1(char *&str, uint8_t code, uint8_t locked_page) noexcept
{
    switch (code) {
    case 0x00: *(str++) = 0; return locked_page;
    case 0x1b: *(str++) = '_'; return locked_page;
    case 0x1c: *(str++) = '.'; return locked_page;
    case 0x1d: *(str++) = '-'; return locked_page;
    case 0x1e: return 0; // S0
    case 0x1f: return 2; // S2
    default:
        *(str++) = (static_cast<char>(code) - 1) + 'A';
        return locked_page;
    }
}

[[nodiscard]] constexpr uint8_t char_from_tt5_page2(char *&str, uint8_t code, uint8_t &locked_page) noexcept
{
    switch (code) {
    case 0x00: *(str++) = 0; return locked_page;
    case 0x0b: *(str++) = ','; return locked_page;
    case 0x0c: *(str++) = ':'; return locked_page;
    case 0x0d: *(str++) = ';'; return locked_page;
    case 0x0e: *(str++) = '/'; return locked_page;
    case 0x0f: *(str++) = '\n'; return locked_page;
    case 0x10: return 0x03; // B0
    case 0x11: return 0x13; // B1
    case 0x12: return 0x23; // B2
    case 0x13: return 0x33; // B3
    case 0x14: return 0x43; // B4
    case 0x15: return 0x53; // B5
    case 0x16: return 0x63; // B6
    case 0x17: return 0x73; // B7
    case 0x18: return locked_page = 0; // L0
    case 0x19: return locked_page = 1; // L1
    case 0x1a: return locked_page = 2; // L2
    case 0x1b: *(str++) = '_'; return locked_page;
    case 0x1c: *(str++) = '.'; return locked_page;
    case 0x1d: *(str++) = '-'; return locked_page;
    case 0x1e: return 0; // S0
    case 0x1f: return 1; // S1
    default:
        *(str++) = (static_cast<char>(code) - 1) + '0';
        return locked_page;
    }
}

[[nodiscard]] constexpr uint8_t char_from_tt5_binary(char *&str, uint8_t code, uint8_t locked_page, uint8_t high_bits) noexcept
{
    *(str++) = (high_bits << 5) | code;
    return locked_page;
}

/** Convert a tt5 value to a string.
 * @param str The buffer to return the string into, there must be enough space.
 */
template<typename T>
constexpr void fill_buffer_from_tt5(char *str, T const &value) noexcept
{
    auto value_ = tt5_reverse(value);

    uint8_t current_page = 0;
    uint8_t locked_page = 0;
    while (value_) {
        auto code = static_cast<uint8_t>(value_ & 0x1f);
        value_ >>= 5;

        switch (current_page & 3) {
        case 0: current_page = char_from_tt5_page0(str, code, locked_page); break;
        case 1: current_page = char_from_tt5_page1(str, code, locked_page); break;
        case 2: current_page = char_from_tt5_page2(str, code, locked_page); break;
        case 3: current_page = char_from_tt5_binary(str, code, locked_page, current_page >> 4); break;
        default: no_default;
        }
    }
    *str = '\0';
}

/** Decode a TT5-packed integer into a UTF-8 string.
* @param value The string packed into an integer, with the last
*         character in the least significant bits.
* @return An UTF-8 encoded string.
*/
template<typename T>
[[nodiscard]] std::string tt5_decode(T const &value) noexcept {
    constexpr ssize_t max_nr_bits = sizeof(T) * CHAR_BIT;
    constexpr ssize_t max_nr_chars = max_nr_bits / 5;

    std::array<char, max_nr_chars + 1> buffer;
    fill_buffer_from_tt5(buffer.data(), value);
    return {buffer.data()};
}

using tt5_64 = uint64_t;
using tt5_128 = ubig128;

/*! _tt5 string literal
*/
constexpr auto operator""_tt5(char const *str, size_t) noexcept
{
    return tt5_encode<tt5_64>(str);
}

/*! _tt5_128 string literal
*/
constexpr auto operator""_tt5_64(char const *str, size_t) noexcept
{
    return tt5_encode<tt5_64>(str);
}

/*! _tt5_128 string literal
*/
constexpr auto operator""_tt5_128(char const *str, size_t) noexcept
{
    return tt5_encode<tt5_128>(str);
}



}