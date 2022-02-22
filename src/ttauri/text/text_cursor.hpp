// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include "../math.hpp"
#include "../cast.hpp"
#include "../unicode/unicode_description.hpp"
#include <tuple>
#include <cstdlib>
#include <algorithm>

namespace tt::inline v1 {

class text_cursor {
public:
    constexpr text_cursor() noexcept : _value(0) {}
    constexpr text_cursor(text_cursor const &) noexcept = default;
    constexpr text_cursor(text_cursor &&) noexcept = default;
    constexpr text_cursor &operator=(text_cursor const &) noexcept = default;
    constexpr text_cursor &operator=(text_cursor &&) noexcept = default;

    /** Create a new text cursor.
    * 
    * @param index The character where the cursor is.
    * @param after True if the cursor is after the character, false if the cursor is before the character
    * @param size The size of the text, used to check for overflow.
    */
    constexpr text_cursor(size_t index, bool after, size_t size) noexcept
    {
        if (size == 0) {
            // Special case, when size is zero, the cursor is before the non-existing first character.
            index = 0;
            after = false;
        } else if (static_cast<ptrdiff_t>(index) < 0) {
            // Underflow.
            index = 0;
            after = false;
        } else if (index >= size) {
            // Overflow.
            index = size - 1;
            after = true;
        }

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
        if (before()) {
            return {index() - 1, true, size};
        } else {
            return {index() + 1, false, size};
        }
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
        return size == 0 or (index() == size - 1 and after()) or index() >= size;
    }

    [[nodiscard]] constexpr size_t index() const noexcept
    {
        return _value >> 1;
    }

    [[nodiscard]] constexpr bool after() const noexcept
    {
        return static_cast<bool>(_value & 1);
    }

    [[nodiscard]] constexpr bool before() const noexcept
    {
        return not after();
    }

    [[nodiscard]] constexpr friend bool operator==(text_cursor const &, text_cursor const &) = default;
    [[nodiscard]] constexpr friend auto operator<=>(text_cursor const &, text_cursor const &) = default;

private:
    size_t _value;
};

} // namespace tt::inline v1
