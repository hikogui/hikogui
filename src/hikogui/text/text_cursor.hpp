// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../unicode/unicode_description.hpp"
#include <tuple>
#include <cstdlib>
#include <algorithm>

namespace hi::inline v1 {

/** A cursor-position in text.
 *
 * The cursor position takes into account the index of the character
 * and if it is in front or behind this character. This allows for more
 * detailed positioning inside bidirectional text.
 *
 */
class text_cursor {
public:
    constexpr text_cursor() noexcept = default;
    constexpr text_cursor(text_cursor const&) noexcept = default;
    constexpr text_cursor(text_cursor&&) noexcept = default;
    constexpr text_cursor& operator=(text_cursor const&) noexcept = default;
    constexpr text_cursor& operator=(text_cursor&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(text_cursor const&, text_cursor const&) = default;
    [[nodiscard]] constexpr friend auto operator<=>(text_cursor const&, text_cursor const&) = default;

    /** Set the text size.
     *
     * This function will clamp the cursor position inside the actual text.
     * It should be called before the cursor position is used to select characters
     * in a text.
     *
     * The cursor position after this function will be:
     * - In the same position if the character still exists in the text.
     * - After the last character in the text if the text is non-empty.
     * - Before the first character in the text if the text is empty.
     *
     * @param size The size of the text string that this cursor points into.
     */
    constexpr text_cursor& resize(size_t size) & noexcept
    {
        inplace_min(_value, max_value(size));
        return *this;
    }

    constexpr text_cursor resize(size_t size) && noexcept
    {
        inplace_min(_value, max_value(size));
        return *this;
    }


    /** Create a new text cursor.
     *
     * @param index The character where the cursor is.
     * @param after True if the cursor is after the character, false if the cursor is before the character
     */
    constexpr text_cursor(size_t index, bool after) noexcept
    {
        _value = (index << 1) | static_cast<size_t>(after);
    }

    /** Return the neighbor cursor.
     *
     * @param size The text size.
     * @return The cursor that is the neighbor of the this cursor.
     *         If this cursor is at start-of-text or end-of-text then this cursor is returned.
     */
    [[nodiscard]] constexpr text_cursor neighbor(size_t size) const noexcept
    {
        auto r = before() ? text_cursor{index() - 1, true} : text_cursor{index() + 1, false};
        return r.resize(size);
    }

    [[nodiscard]] constexpr text_cursor after_neighbor(size_t size) const noexcept
    {
        return before() ? neighbor(size) : *this;
    }

    [[nodiscard]] constexpr text_cursor before_neighbor(size_t size) const noexcept
    {
        return after() ? neighbor(size) : *this;
    }

    [[nodiscard]] constexpr bool start_of_text() const noexcept
    {
        return _value == 0;
    }

    [[nodiscard]] constexpr bool end_of_text(size_t size) const noexcept
    {
        return _value >= max_value(size);
    }

    [[nodiscard]] constexpr size_t index() const noexcept
    {
        return _value >> 1;
    }

    [[nodiscard]] constexpr bool after() const noexcept
    {
        return to_bool(_value & 1);
    }

    [[nodiscard]] constexpr bool before() const noexcept
    {
        return not after();
    }

private:
    /** Get the max value of the cursor for the text size.
    * 
    * @param size The size of text in number of characters.
    * @return The maximum value that _value may have.
    */
    [[nodiscard]] constexpr static size_t max_value(size_t size) noexcept
    {
        if (size) {
            size <<= 1;
            --size;
        }
        return size;
    }

    /**
     *
     * Bits:
     *  - [n:1] The index of the character.
     *  - [0] '1': cursor behind character. '0' cursor in front of character
     *
     * If zero then this is in front of the first character in the text;
     * zero can therefor be used as a cursor position in an empty text.
     *
     */
    size_t _value = 0;
};

} // namespace hi::inline v1
