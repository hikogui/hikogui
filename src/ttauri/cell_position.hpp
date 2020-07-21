// Copyright 2020 Pokitec
// All rights reserved.

#include "required.hpp"

namespace tt {

/** A position and size of a cell.
 * [63] Absolute row
 * [62] Absolute column
 * [61] Negative row
 * [60] Negative column
 * [47:40] rowspan (0 == copy or 1)
 * [39:32] colspan (0 == copy or 1)
 * [31:16] row
 * [15:0] column
 */
enum class cell_position : uint64_t {};

namespace detail {
constexpr int cell_position_absolute_shift = 62;
constexpr int cell_position_negative_shift = 61;
constexpr int cell_position_span_shift = 32;
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_absolute(cell_position const &position) noexcept
{
    constexpr auto shift = cell_position_absolute_shift + static_cast<int>(IsRow);

    return static_cast<bool>(
        static_cast<uint64_t>(position) >> shift & uint64_t{1}
    );
}

template<bool IsRow>
[[nodiscard]] constexpr void set_absolute(cell_position &position, bool value) noexcept
{
    constexpr auto shift = cell_position_absolute_shift + static_cast<int>(IsRow);

    position &= ~(uint64_t{1} << shift);
    position |= static_cast<uint64_t>(value) << shift;
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_negative(cell_position const &position) noexcept
{
    constexpr auto shift = cell_position_negative_shift + static_cast<int>(IsRow);

    return static_cast<bool>(
        static_cast<uint64_t>(position) >> shift & uint64_t{1}
    );
}

template<bool IsRow>
[[nodiscard]] constexpr bool is_positive(cell_position const &position) noexcept
{
    return !is_negative<IsRow>(position);
}

template<bool IsRow>
[[nodiscard]] constexpr void set_absolute(cell_position &position, bool value) noexcept
{
    constexpr auto shift = cell_position_negative_shift + static_cast<int>(IsRow);

    position &= ~(uint64_t{1} << shift);
    position |= static_cast<uint64_t>(value) << shift;
}

template<bool IsRow>
[[nodiscard]] constexpr int get_span(cell_position const &position) noexcept
{
    constexpr auto shift = cell_position_span_shift + static_cast<int>(IsRow) * 8;

    return static_cast<int>(
        static_cast<uint64_t>(position) >> shift & uint64_t{0xff}
    );
}

template<bool IsRow>
[[nodiscard]] constexpr void set_span(cell_position &position, int value) noexcept
{
    tt_assume(value >= 0);
    constexpr auto shift = cell_position_span_shift + static_cast<int>(IsRow) * 8;
    position &= ~(uint64_t{0xff} << shift);
    position |= static_cast<uint64_t>(value) << shift;
}

template<bool IsRow>
[[nodiscard]] constexpr int get_coord(cell_position const &position) noexcept
{
    constexpr auto shift = static_cast<int>(IsRow) * 16;

    return static_cast<int>(
        static_cast<uint64_t>(position) >> shift & uint64_t{0xffff}
    );
}

template<bool IsRow>
[[nodiscard]] constexpr void set_coord(cell_position &position, int value) noexcept
{
    tt_assume(value >= 0);
    constexpr auto shift = static_cast<int>(IsRow) * 16;

    position &= ~(uint64_t{0xffff} << shift);
    position |= static_cast<uint64_t>(value) << shift;
}

/** Parse a cell position
 *
 * cell_position := row | col | rowspan | colspan;
 * row := 'r' (relative | absolute);
 * col := 'c' (relative | absolute);
 * rowspan := 'rs' number
 * colspan := 'cs' number
 * relative := '+' number | '-' number
 * absolute := '<'? number | '>' number
 * number := [0-9]+
 */
[[nodiscard]] constexpr cell_position parse_cell_position(char const *str) noexcept
{
    enum class state_t { Idle, Start, Number };
    
    bool is_row;
    bool is_span;
    bool is_absolute;
    bool is_negative;
    uint64_t value;

    auto state = state_t::Idle;
    auto position = static_cast<cell_position>(0);
    char c;
    do {
        c = *str;
        auto consume = true;

        switch (state) {
        case state_t::Idle:
            switch (c) {
            case 'r': state = state_t::Start; is_row = true; break;
            case 'c': state = state_t::Start; is_row = false; break;
            case ' ': break;
            case 0:
                // End of the string. Don't consume the nul.
                consume = false;
                break;
            default: tt_no_default;
            }
            break;
        
        case state_t::Start:
            is_span = false;
            is_absolute = false;
            is_negative = false;
            value = false;
            state = state_t::Number;
            switch (c) {
            case 's': is_absolute = true; is_span = true; break;
            case '+': break;
            case '-': is_negative = true; break;
            case '<': is_absolute = true; break;
            case '>': is_absolute = true; is_negative = true; break;
            case ' ': state = state_t::Start; break;
            default:
                // This is already the first digit, switch to the next state
                // without consuming this character.
                is_absolute = true;
                consume = false;
            }
            break;

        case state_t::Number:
            value *= 10;
            if (c >= '0' && c <= '9') {
                value += static_cast<uint64_t>(c - '0');

            } else {
                // The first non-digit character (including '\0') is the new
                // command or the end of the string. Switch the Idle and
                // don't consume the character.
                if (is_span) {
                    if (is_row) {
                        set_span<true>(position, value);
                    } else {
                        set_span<false>(position, value);
                    }
                    
                } else {
                    if (is_row) {
                        set_coord<true>(position, value);
                        set_negative<true>(is_negative);
                        set_absolute<true>(is_absolute);
                    } else {
                        set_coord<false>(position, value);
                        set_negative<false>(is_negative);
                        set_absolute<false>(is_negative);
                    }
                }

                consume = false;
                state = state_t::Idle;
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
constexpr void transform_half(cell_position &r, cell_position const &lhs, cell_position const &rhs) noexcept
{
    if (get_span<IsRow>(lhs) != 0) {
        set_span<IsRow>(r, get_span<IsRow>(lhs));
    } else {
        set_span<IsRow>(r, get_span<IsRow>(rhs));
    }

    if (is_absolute<IsRow>(lhs)) {
        set_absolute<IsRow>(r, true);
        set_negative<IsRow>(r, is_negative<IsRow>(lhs));
        set_coord<IsRow>(r, get_coord<IsRow>(lhs));

    } else if (is_absolute<IsRow>(rhs)) {
        set_absolute<IsRow>(r, true);
        set_negative<IsRow>(r, is_negative<IsRow>(rhs));
        if (get_coord<IsRow>(lhs) == 0) {
            set_coord<IsRow>(r, get_coord<IsRow>(rhs));

        } else if (is_negative<IsRow>(lhs)) {
            set_coord<IsRow>(r, get_coord<IsRow>(rhs) - get_coord<IsRow>(lhs));
        } else {
            set_coord<IsRow>(r, get_coord<IsRow>(rhs) + get_coord<IsRow>(lhs));
        }

    } else {
        ttlet lhs_coord = is_negative<IsRow>(lhs) ? -get_coord<IsRow>(lhs) : get_coord<IsRow>(lhs);
        ttlet rhs_coord = is_negative<IsRow>(rhs) ? -get_coord<IsRow>(rhs) : get_coord<IsRow>(rhs);
        ttlet r_coord = rhs_coord + lhs_coord;
        set_absolute<IsRow>(r, false);
        set_negative<IsRow>(r, r_coord < 0);
        set_coord<IsRow>(r, std::abs(r_coord));
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

}
