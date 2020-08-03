// Copyright 2020 Pokitec
// All rights reserved.

#include "required.hpp"
#include "numeric_cast.hpp"
#include "exceptions.hpp"

#pragma once

namespace tt {

namespace detail {
constexpr int cell_address_absolute_shift = 62;
constexpr int cell_address_opposite_shift = 60;
constexpr int cell_address_span_shift = 32;
constexpr int cell_address_alignment_shift = 44;
}

/** The absolute or relative position and size of a cell.
 * There are four quadrants where cells can be located, using
 * the opposite flag you can select the opposite quadrant for
 * the row or column.
 *
 * The address can be absolute, or relative compared to another address.
 *
 * The address also contains a row- and colspan.
 * 
 *  Bits    | Type    | Description
 * :--------|:--------|:------------
 *  [63]    | bool    | Absolute row
 *  [62]    | bool    | Absolute column
 *  [61]    | bool    | Opposite row (true=top, false=bottom)
 *  [60]    | bool    | Opposite column (true=right, false=left)
 *  [55:50] | uint6_t | Row alignment offset
 *  [49:44] | uint6_t | Column alignment offset
 *  [43:38] | uint6_t | rowspan - 1
 *  [37:32] | uint6_t | colspan - 1
 *  [31:16] | int16_t | row (must be natural for absolute row)
 *  [15: 0] | int16_t | column (must be natural for absolute column)
 */
enum class cell_address : uint64_t {};

template<bool IsRow>
[[nodiscard]] constexpr bool is_absolute(cell_address const &position) noexcept
{
    constexpr auto shift = detail::cell_address_absolute_shift + static_cast<int>(IsRow);

    return static_cast<bool>(
        static_cast<uint64_t>(position) >> shift & uint64_t{1}
    );
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_relative(cell_address const &position) noexcept
{
    return !is_absolute<IsRow>(position);
}

template<bool IsRow>
constexpr void set_absolute(cell_address &position, bool value) noexcept
{
    constexpr auto shift = detail::cell_address_absolute_shift + static_cast<int>(IsRow);

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{1} << shift);
    position_ |= static_cast<uint64_t>(value) << shift;
    position = static_cast<cell_address>(position_);
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_opposite(cell_address const &position) noexcept
{
    constexpr auto shift = detail::cell_address_opposite_shift + static_cast<int>(IsRow);

    return static_cast<bool>(
        static_cast<uint64_t>(position) >> shift & uint64_t{1}
    );
}

template<bool IsRow>
constexpr void set_opposite(cell_address &position, bool value) noexcept
{
    constexpr auto shift = detail::cell_address_opposite_shift + static_cast<int>(IsRow);

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{1} << shift);
    position_ |= static_cast<uint64_t>(value) << shift;
    position = static_cast<cell_address>(position_);
}

template<bool IsRow>
[[nodiscard]] constexpr int get_alignment(cell_address const &position) noexcept
{
    constexpr auto shift = detail::cell_address_alignment_shift + static_cast<int>(IsRow) * 6;

    return static_cast<int>(static_cast<uint64_t>(position) >> shift & 0x3f);
}

template<bool IsRow>
constexpr void set_alignment(cell_address &position, int value) noexcept
{
    tt_assume(value >= 0 && value < 64);
    ttlet value_ = static_cast<uint64_t>(value);

    constexpr auto shift = detail::cell_address_alignment_shift + static_cast<int>(IsRow) * 6;

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{0x3f} << shift);
    position_ |= value_ << shift;
    position = static_cast<cell_address>(position_);
}

template<bool IsRow>
[[nodiscard]] constexpr int get_span(cell_address const &position) noexcept
{
    constexpr auto shift = detail::cell_address_span_shift + static_cast<int>(IsRow) * 6;

    return static_cast<int>((static_cast<uint64_t>(position) >> shift & 0x3f) + 1);
}

template<bool IsRow>
constexpr void set_span(cell_address &position, int value) noexcept
{
    tt_assume(value >= 1 && value < 65);
    ttlet value_ = static_cast<uint64_t>(value - 1);

    constexpr auto shift = detail::cell_address_span_shift + static_cast<int>(IsRow) * 6;

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{0x3f} << shift);
    position_ |= value_ << shift;
    position = static_cast<cell_address>(position_);
}

template<bool IsRow>
[[nodiscard]] constexpr int get_coord(cell_address const &position) noexcept
{
    constexpr auto shift = static_cast<int>(IsRow) * 16;

    return static_cast<int16_t>(
        static_cast<uint16_t>(static_cast<uint64_t>(position) >> shift)
    );
}

template<bool IsRow>
constexpr void set_coord(cell_address &position, int value) noexcept
{
    ttlet value_ = static_cast<uint16_t>(value);

    constexpr auto shift = static_cast<int>(IsRow) * 16;

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{0xffff} << shift);
    position_ |= static_cast<uint64_t>(static_cast<uint16_t>(value_)) << shift;
    position = static_cast<cell_address>(position_);
}

/** Parse a cell position
 *
 * cell_address := position*;
 * position := axis ([+-]? number)? (':' number (':' number)?)?;
 * axis := [BbTtLlRr]
 * number := [0-9]+
 */
[[nodiscard]] constexpr cell_address parse_cell_address(char const *str)
{
    enum class state_t { Idle, Coord, Number };
    enum class part_t { Coord, Span, Alignment };

    char axis = 0;
    part_t part = part_t::Coord;
    bool is_absolute = false;
    bool is_positive = true;
    int value = 0;

    auto state = state_t::Idle;
    auto position = static_cast<cell_address>(0);
    char c = 0;
    do {
        c = *str;
        auto consume = true;

        switch (state) {
        case state_t::Idle:
            value = 0;
            is_positive = true;
            part = part_t::Coord;
            is_absolute = true;

            switch (c) {
            case 'L':
            case 'l':
                state = state_t::Coord;
                axis = 'L';
                break;
            case 'R':
            case 'r':
                state = state_t::Coord;
                axis = 'R';
                break;
            case 'B':
            case 'b':
                state = state_t::Coord;
                axis = 'B';
                break;
            case 'T':
            case 't':
                state = state_t::Coord;
                axis = 'T';
                break;
            case ' ':
                break;
            case 0:
                // End of the string. Don't consume the nul.
                consume = false;
                break;
            default:
                TTAURI_THROW(parse_error("Unexpected character"));
            }
            break;
        
        case state_t::Coord:
            switch (c) {
            case '+':
                state = state_t::Number;
                is_absolute = false;
                is_positive = true;
                break;
            case '-':
                state = state_t::Number;
                is_absolute = false;
                is_positive = false;
                break;
            case ':':
                state = state_t::Number;
                part = part_t::Span;
                break;
            case ' ':
                break;
            default:
                // This is already the first digit, switch to the next state
                // without consuming this character.
                state = state_t::Number;
                consume = false;
            }
            break;

        case state_t::Number:
            if (c >= '0' && c <= '9') {
                value *= 10;
                value += static_cast<int>(c - '0');

            } else {
                if (!is_positive) {
                    value = -value;
                }

                // The first non-digit character (including '\0') is the new
                // command or the end of the string. Switch the Idle and
                // don't consume the character.
                ttlet is_row = axis == 'B' || axis == 'T';
                ttlet is_opposite = axis == 'R' || axis == 'T';
                switch (part) {
                case part_t::Coord:
                    if (is_row) {
                        set_coord<true>(position, value);
                        set_opposite<true>(position, is_opposite);
                        set_absolute<true>(position, is_absolute);
                    } else {
                        set_coord<false>(position, value);
                        set_opposite<false>(position, is_opposite);
                        set_absolute<false>(position, is_absolute);
                    }
                    break;

                case part_t::Span:
                    if (is_row) {
                        set_span<true>(position, value);
                        set_opposite<true>(position, is_opposite);
                    } else {
                        set_span<false>(position, value);
                        set_opposite<false>(position, is_opposite);
                    }
                    break;

                case part_t::Alignment:
                    if (is_row) {
                        set_alignment<true>(position, value);
                    } else {
                        set_alignment<false>(position, value);
                    }
                    break;

                default:
                    tt_no_default;
                }

                if (c == ':') {
                    switch (part) {
                    case part_t::Coord:
                        // A ':' after a coord means a span, parse the next number.
                        value = 0;
                        part = part_t::Span;
                        is_positive = true;
                        state = state_t::Number;
                        break;

                    case part_t::Span:
                        // A ':' after a span means an alignment, parse the next number.
                        value = 0;
                        part = part_t::Alignment;
                        is_positive = true;
                        state = state_t::Number;
                        break;

                    case part_t::Alignment:
                        TTAURI_THROW(parse_error("Unexpected third ':'"));

                    default:
                        tt_no_default;
                    }

                } else {
                    // Any other character means we start over, do not consume this character.
                    consume = false;
                    state = state_t::Idle;
                }
            }
            break;

        default:
            tt_no_default;
        }

        if (consume) {
            ++str;
        }
    } while (c != 0);

    return position;
}

template<bool IsRow>
[[nodiscard]] std::string to_string_half(cell_address const &rhs) noexcept
{
    auto r = std::string{};

    auto axis = IsRow ?
        (is_opposite<IsRow>(rhs) ? 'T' : 'B') :
        (is_opposite<IsRow>(rhs) ? 'R' : 'L');

    if (is_relative<IsRow>(rhs)) {
        if (auto coord = get_coord<IsRow>(rhs); coord != 0) {
            r += fmt::format("{}{:+}", axis, coord);
        }
    } else {
        auto coord = get_coord<IsRow>(rhs);
        r += fmt::format("{}{}", axis, coord);
    }

    if (auto span = get_span<IsRow>(rhs); span != 1) {
        if (nonstd::ssize(r) == 0) {
            r += axis;
        }
        r += fmt::format(":{}", span);

        if (auto alignment = get_alignment<IsRow>(rhs); alignment != 0) {
            r += fmt::format(":{}", alignment);
        }
    }

    return r;
}

[[nodiscard]] inline std::string to_string(cell_address const &rhs) noexcept
{
    return to_string_half<false>(rhs) + to_string_half<true>(rhs);
}

inline std::ostream &operator<<(std::ostream &lhs, cell_address const &rhs)
{
    return lhs << to_string(rhs);
}

template<bool IsRow>
constexpr void transform_half(cell_address &r, cell_address const &lhs, cell_address const &rhs) noexcept
{
    set_span<IsRow>(r, get_span<IsRow>(lhs));
    set_alignment<IsRow>(r, get_alignment<IsRow>(lhs));

    if (is_absolute<IsRow>(lhs)) {
        set_absolute<IsRow>(r, true);
        set_opposite<IsRow>(r, is_opposite<IsRow>(lhs));
        set_coord<IsRow>(r, get_coord<IsRow>(lhs));

    } else {
        set_absolute<IsRow>(r, is_absolute<IsRow>(rhs));
        set_opposite<IsRow>(r, is_opposite<IsRow>(rhs));
        if (is_opposite<IsRow>(lhs) == is_opposite<IsRow>(rhs)) {
            set_coord<IsRow>(r, get_coord<IsRow>(rhs) + get_coord<IsRow>(lhs));
        } else {
            set_coord<IsRow>(r, get_coord<IsRow>(rhs) - get_coord<IsRow>(lhs));
        }
    }
}

/** Transform rhs by lhs.
 */
[[nodiscard]] constexpr cell_address operator*(cell_address const &lhs, cell_address const &rhs) noexcept
{
    auto r = static_cast<cell_address>(0);
    transform_half<true>(r, lhs, rhs);
    transform_half<false>(r, lhs, rhs);
    return r;
}

/** Transform lhs/this by rhs.
 */
constexpr cell_address &operator*=(cell_address &lhs, cell_address const &rhs) noexcept
{
    lhs = rhs * lhs;
    return lhs;
}

/** Find the begin of the cell.
* @param IsRow True for row, False for column
* @param cell The cell to find the begin of
* @param size The total size of the table.
* @return The begin position at the postive or negative side of the table.
*         If the size is given then the negative side is mapped to the positive
*         side of the table.
*/
template<bool IsRow>
[[nodiscard]] int begin(cell_address const &rhs, int size=0) noexcept
{
    if (size == 0 || !is_opposite<IsRow>(rhs)) {
        return get_coord<IsRow>(rhs);

    } else {
        return size - get_coord<IsRow>(rhs) - std::max(1, get_span<IsRow>(rhs));
    }
}

/** Find one beyond the end of the cell.
* @param IsRow True for row, False for column
* @param cell The cell to find the end of
* @param size The total size of the table.
* @return One beyond the end position at the postive or negative side of the table.
*         If the size is given then the negative side is mapped to the positive
*         side of the table.
*/
template<bool IsRow>
[[nodiscard]] int end(cell_address const &rhs, int size=0) noexcept
{
    if (size == 0 || !is_opposite<IsRow>(rhs)) {
        return get_coord<IsRow>(rhs) + std::max(1, get_span<IsRow>(rhs));

    } else {
        return size - get_coord<IsRow>(rhs);
    }
}

template<typename It>
[[nodiscard]] constexpr std::tuple<int,int,int,int> cell_address_max(It const &first, It const &last) noexcept
{
    auto max_from_left = 0;
    auto max_from_right = 0;
    auto max_from_bottom = 0;
    auto max_from_top = 0;
    for (auto i = first; i != last; ++i) {
        tt_assume(is_absolute<false>(*i));
        tt_assume(is_absolute<true>(*i));

        if (is_opposite<false>(*i)) {
            max_from_right = std::max(max_from_right, end<false>(*i));
        } else {
            max_from_left = std::max(max_from_left, end<false>(*i));
        }
        if (is_opposite<true>(*i)) {
            max_from_top = std::max(max_from_top, end<true>(*i));
        } else {
            max_from_bottom = std::max(max_from_bottom, end<true>(*i));
        }
    }

    return {max_from_left, max_from_right, max_from_bottom, max_from_top};
}

constexpr cell_address operator "" _ca(char const *str, size_t str_len) noexcept {
    return parse_cell_address(str);
}


}
