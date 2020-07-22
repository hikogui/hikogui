// Copyright 2020 Pokitec
// All rights reserved.

#include "required.hpp"

namespace tt {

/** A position and size of a cell.
 * 
 *  Bits    | Type    | Description
 * :--------|:--------|:------------
 *  [63]    | bool    | Absolute row
 *  [62]    | bool    | Absolute column
 *  [61]    | bool    | Opposite row (true=top, false=bottom)
 *  [60]    | bool    | Opposite column (true=right, false=left)
 *  [47:40] | uint8_t | rowspan - 1
 *  [39:32] | uint8_t | colspan - 1
 *  [31:16] | int16_t | row (must be natural for absolute row)
 *  [15: 0] | int16_t | column (must be natural for absolute column)
 */
enum class cell_position : uint64_t {};

namespace detail {
constexpr int cell_position_absolute_shift = 62;
constexpr int cell_position_opposite_shift = 60;
constexpr int cell_position_span_shift = 32;
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_absolute(cell_position const &position) noexcept
{
    constexpr auto shift = detail::cell_position_absolute_shift + static_cast<int>(IsRow);

    return static_cast<bool>(
        static_cast<uint64_t>(position) >> shift & uint64_t{1}
    );
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_relative(cell_position const &position) noexcept
{
    return !is_absolute<IsRow>(position);
}

template<bool IsRow>
[[nodiscard]] constexpr void set_absolute(cell_position &position, bool value) noexcept
{
    constexpr auto shift = detail::cell_position_absolute_shift + static_cast<int>(IsRow);

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{1} << shift);
    position_ |= static_cast<uint64_t>(value) << shift;
    position = static_cast<cell_position>(position_);
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_opposite(cell_position const &position) noexcept
{
    constexpr auto shift = detail::cell_position_opposite_shift + static_cast<int>(IsRow);

    return static_cast<bool>(
        static_cast<uint64_t>(position) >> shift & uint64_t{1}
    );
}

template<bool IsRow>
[[nodiscard]] constexpr void set_opposite(cell_position &position, bool value) noexcept
{
    constexpr auto shift = detail::cell_position_opposite_shift + static_cast<int>(IsRow);

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{1} << shift);
    position_ |= static_cast<uint64_t>(value) << shift;
    position = static_cast<cell_position>(position_);
}

template<bool IsRow>
[[nodiscard]] constexpr uint8_t get_span(cell_position const &position) noexcept
{
    constexpr auto shift = detail::cell_position_span_shift + static_cast<int>(IsRow) * 8;

    return static_cast<uint8_t>(static_cast<uint64_t>(position) >> shift) + 1;
}

template<bool IsRow>
[[nodiscard]] constexpr void set_span(cell_position &position, uint8_t value) noexcept
{
    tt_assume(value >= 1);
    constexpr auto shift = detail::cell_position_span_shift + static_cast<int>(IsRow) * 8;

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{0xff} << shift);
    position_ |= static_cast<uint64_t>(value - 1) << shift;
    position = static_cast<cell_position>(position_);
}

template<bool IsRow>
[[nodiscard]] constexpr int16_t get_coord(cell_position const &position) noexcept
{
    constexpr auto shift = static_cast<int>(IsRow) * 16;

    return static_cast<int16_t>(
        static_cast<uint16_t>(static_cast<uint64_t>(position) >> shift)
    );
}

template<bool IsRow>
[[nodiscard]] constexpr void set_coord(cell_position &position, int16_t value) noexcept
{
    constexpr auto shift = static_cast<int>(IsRow) * 16;

    auto position_ = static_cast<uint64_t>(position);
    position_ &= ~(uint64_t{0xffff} << shift);
    position_ |= static_cast<uint64_t>(static_cast<uint16_t>(value)) << shift;
    position = static_cast<cell_position>(position_);
}

/** Parse a cell position
 *
 * cell_position := position*;
 * position := axis ([+-]? number)? (':' number)?;
 * axis := [BbTtLlRr]
 * number := [0-9]+
 */
[[nodiscard]] constexpr cell_position parse_cell_position(char const *str) noexcept
{
    enum class state_t { Idle, Coord, Number };
    
    char axis = 0;
    bool is_span = false;
    bool is_absolute = false;
    bool is_positive = true;
    int value = 0;

    auto state = state_t::Idle;
    auto position = static_cast<cell_position>(0);
    char c = 0;
    do {
        c = *str;
        auto consume = true;

        switch (state) {
        case state_t::Idle:
            value = 0;
            is_positive = true;
            is_span = false;
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
            default: tt_no_default;
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
                is_span = true;
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
                if (is_span) {
                    if (is_row) {
                        set_span<true>(position, numeric_cast<uint8_t>(value));
                        set_opposite<true>(position, is_opposite);
                    } else {
                        set_span<false>(position, numeric_cast<uint8_t>(value));
                        set_opposite<false>(position, is_opposite);
                    }
                    
                } else {
                    if (is_row) {
                        set_coord<true>(position, numeric_cast<int16_t>(value));
                        set_opposite<true>(position, is_opposite);
                        set_absolute<true>(position, is_absolute);
                    } else {
                        set_coord<false>(position, numeric_cast<int16_t>(value));
                        set_opposite<false>(position, is_opposite);
                        set_absolute<false>(position, is_absolute);
                    }
                }

                if (is_span == false && c == ':') {
                    // A ':' after a number means a span, parse the next number.
                    value = 0;
                    is_span = true;
                    is_positive = true;
                    state = state_t::Number;

                } else {
                    // Any other character means we start over, do not consume this
                    // character.
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
[[nodiscard]] std::string to_string_half(cell_position const &rhs) noexcept
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
    }

    return r;
}

[[nodiscard]] std::string to_string(cell_position const &rhs) noexcept
{
    return to_string_half<false>(rhs) + to_string_half<true>(rhs);
}

std::ostream &operator<<(std::ostream &lhs, cell_position const &rhs)
{
    return lhs << to_string(rhs);
}

template<bool IsRow>
constexpr void transform_half(cell_position &r, cell_position const &lhs, cell_position const &rhs) noexcept
{
    set_span<IsRow>(r, get_span<IsRow>(lhs));

    if (is_absolute<IsRow>(lhs)) {
        set_absolute<IsRow>(r, true);
        set_opposite<IsRow>(r, is_opposite<IsRow>(lhs));
        set_coord<IsRow>(r, get_coord<IsRow>(lhs));

    } else if (is_absolute<IsRow>(rhs)) {
        set_absolute<IsRow>(r, true);
        set_opposite<IsRow>(r, is_opposite<IsRow>(rhs));
        set_coord<IsRow>(r, get_coord<IsRow>(rhs) + get_coord<IsRow>(lhs));

    } else {
        set_absolute<IsRow>(r, false);
        set_opposite<IsRow>(r, is_opposite<IsRow>(lhs));
        set_coord<IsRow>(r, get_coord<IsRow>(rhs) + get_coord<IsRow>(lhs));
    }
}

/** Transform rhs by lhs.
 */
[[nodiscard]] constexpr cell_position operator*(cell_position const &lhs, cell_position const &rhs) noexcept
{
    auto r = static_cast<cell_position>(0);
    transform_half<true>(r, lhs, rhs);
    transform_half<false>(r, lhs, rhs);
    return r;
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
[[nodiscard]] int begin(cell_position const &rhs, int size=0) noexcept
{
    if (size == 0 || is_positive<IsRow>(rhs)) {
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
[[nodiscard]] int end(cell_position const &rhs, int size=0) noexcept
{
    if (size == 0 || is_positive<IsRow>(rhs)) {
        return get_coord<IsRow>(rhs) + std::max(1, get_span<IsRow>(rhs));

    } else {
        return size - get_coord<IsRow>(rhs);
    }
}

template<typename It>
[[nodiscard]] constexpr std::tuple<int,int,int,int> cell_position_max(It const &first, It const &last) noexcept
{
    auto max_from_left = 0;
    auto max_from_right = 0;
    auto max_from_bottom = 0;
    auto max_from_top = 0;
    for (auto i = first; i != last; ++i) {
        tt_assume(is_absolute<false>(*i));
        tt_assume(is_absolute<true>(*i));

        if (is_negative<false>(*i)) {
            max_from_right = std::max(max_from_right, end<false>(*i));
        } else {
            max_from_left = std::max(max_from_left, end<false>(*i));
        }
        if (is_negative<true>(*i)) {
            max_from_right = std::max(max_from_top, end<true>(*i));
        } else {
            max_from_left = std::max(max_from_bottom, end<true>(*i));
        }
    }

    return {max_from_left, max_from_right, max_from_bottom, max_from_top};
}

constexpr cell_position operator "" _cp(char const *str, size_t str_len) noexcept {
    return parse_cell_position(str);
}


}
