// Copyright 2020 Pokitec
// All rights reserved.

#include "required.hpp"
#include "cast.hpp"
#include "exceptions.hpp"
#include "alignment.hpp"

#pragma once

namespace tt {

template<arrangement Arrangement>
struct cell_address_axis {
    bool is_absolute;
    bool is_opposite;
    int8_t alignment;
    int8_t span;
    int16_t index;

    [[nodiscard]] constexpr cell_address_axis() noexcept :
        is_absolute(false), is_opposite(false), alignment(0), span(1), index(0) {}

    /** Find the begin of the cell.
     * @param size The total size of the axis.
     * @return The first tick number belonging to this address
     */
    [[nodiscard]] ssize_t begin(ssize_t size) const noexcept
    {
        tt_assume(is_absolute);
        tt_assume(span >= 1);
        tt_assume(index >= 0);

        ttlet index_ = (size == 0 || !is_opposite) ? index : size - index - span;

        tt_assume(index_ >= 0 && index_ < size);
        return index_;
    }

    /** Find one beyond the end of the cell.
     * @param size The total size of the axis.
     * @return One beyond the last the tick number belonging to this address
     */
    [[nodiscard]] ssize_t end(ssize_t size) const noexcept
    {
        tt_assume(is_absolute);
        tt_assume(span >= 1);
        tt_assume(index >= 0);

        ttlet index_span = (size == 0 || !is_opposite) ? index + span : size - index;
        
        tt_assume(index_span >= 1 && index_span <= size);
        return index_span;
    }

    /** Find on which cell this aligns to.
     * @param size The total size of the axis.
     * @return The tick number on the axis where the address is aligned to
     */
    [[nodiscard]] ssize_t aligned_to(ssize_t size) const noexcept
    {
        tt_assume(alignment >= 0 && alignment < span);

        ttlet aligned_to_ = begin(size) + alignment;

        tt_assume(aligned_to_ >= 0 && aligned_to_ < size);
        return aligned_to_;
    }

    [[nodiscard]] constexpr bool operator==(cell_address_axis const &rhs) const noexcept
    {
        tt_assume(span >= 1);
        tt_assume(alignment >= 0);
        tt_assume(rhs.span >= 1);
        tt_assume(rhs.alignment >= 0);

        return
            is_absolute == rhs.is_absolute &&
            is_opposite == rhs.is_opposite &&
            index == rhs.index &&
            span == rhs.span &&
            alignment == rhs.alignment;
    }


    [[nodiscard]] friend constexpr cell_address_axis
    operator*(cell_address_axis const &lhs, cell_address_axis const &rhs) noexcept
    {
        cell_address_axis r;

        tt_assume(lhs.span >= 1);
        r.span = lhs.span;

        tt_assume(lhs.alignment >= 0);
        r.alignment = lhs.alignment;

        if (lhs.is_absolute) {
            r.is_absolute = true;
            r.is_opposite = lhs.is_opposite;
            r.index = lhs.index;

        } else {
            r.is_absolute = rhs.is_absolute;
            r.is_opposite = rhs.is_opposite;
            if (lhs.is_opposite == rhs.is_opposite) {
                r.index = rhs.index + lhs.index;
            } else {
                r.index = rhs.index - lhs.index;
            }
        }

        return r;
    }

    [[nodiscard]] friend std::string to_string(cell_address_axis const &rhs) noexcept
    {
        auto r = std::string{};

        auto axis = Arrangement == arrangement::row ? (rhs.is_opposite ? 'T' : 'B') : (rhs.is_opposite ? 'R' : 'L');

        if (!rhs.is_absolute) {
            if (rhs.index != 0) {
                r += fmt::format("{}{:+}", axis, rhs.index);
            }
        } else {
            r += fmt::format("{}{}", axis, rhs.index);
        }

        tt_assume(rhs.span >= 1);
        if (rhs.span != 1) {
            if (std::ssize(r) == 0) {
                r += axis;
            }
            r += fmt::format(":{}", rhs.span);

            tt_assume(rhs.alignment >= 0);
            if (rhs.alignment != 0) {
                r += fmt::format(":{}", rhs.alignment);
            }
        }

        return r;
    }

    friend struct cell_address;
};

struct cell_address {
    cell_address_axis<arrangement::row> row;
    cell_address_axis<arrangement::column> column;

    [[nodiscard]] constexpr cell_address() noexcept : row(), column() {}

    /** Parse a cell position
     *
     * cell_address := position*;
     * position := axis ([+-]? number)? (':' number (':' number)?)?;
     * axis := [BbTtLlRr]
     * number := [0-9]+
     */
    [[nodiscard]] constexpr cell_address(char const *str) : row(), column()
    {
        enum class state_t { Idle, Coord, Number };
        enum class part_t { Coord, Span, Alignment };

        char axis = 0;
        part_t part = part_t::Coord;
        bool is_absolute = false;
        bool is_positive = true;
        int value = 0;

        auto state = state_t::Idle;
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
                case ' ': break;
                case 0:
                    // End of the string. Don't consume the nul.
                    consume = false;
                    break;
                default: TTAURI_THROW(parse_error("Unexpected character"));
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
                case ' ': break;
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
                            row.index = narrow_cast<int16_t>(value);
                            row.is_opposite = is_opposite;
                            row.is_absolute = is_absolute;
                        } else {
                            column.index = narrow_cast<int16_t>(value);
                            column.is_opposite = is_opposite;
                            column.is_absolute = is_absolute;
                        }
                        break;

                    case part_t::Span:
                        if (is_row) {
                            row.span = narrow_cast<int8_t>(value);
                            row.is_opposite = is_opposite;
                        } else {
                            column.span = narrow_cast<int8_t>(value);
                            column.is_opposite = is_opposite;
                        }
                        break;

                    case part_t::Alignment:
                        if (is_row) {
                            row.alignment = narrow_cast<int8_t>(value);
                        } else {
                            column.alignment = narrow_cast<int8_t>(value);
                        }
                        break;

                    default: tt_no_default();
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

                        default: tt_no_default();
                        }

                    } else {
                        // Any other character means we start over, do not consume this character.
                        consume = false;
                        state = state_t::Idle;
                    }
                }
                break;

            default: tt_no_default();
            }

            if (consume) {
                ++str;
            }
        } while (c != 0);
    }

    /** Transform lhs/this by rhs.
     */
    constexpr cell_address &operator*=(cell_address const &rhs) noexcept
    {
        *this = rhs * *this;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(cell_address const &rhs) const noexcept
    {
        return this->row == rhs.row && this->column == rhs.column;
    }

    /** Transform rhs by lhs.
     */
    [[nodiscard]] friend constexpr cell_address operator*(cell_address const &lhs, cell_address const &rhs) noexcept
    {
        cell_address r;
        r.row = lhs.row * rhs.row;
        r.column = lhs.column * rhs.column;
        return r;
    }

    
    [[nodiscard]] friend std::string to_string(cell_address const &rhs) noexcept
    {
        return to_string(rhs.column) + to_string(rhs.row);
    }

    friend std::ostream &operator<<(std::ostream &lhs, cell_address const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

[[nodiscard]] constexpr cell_address operator"" _ca(char const *str, size_t str_len) noexcept
{
    return cell_address{str};
}

} // namespace tt
