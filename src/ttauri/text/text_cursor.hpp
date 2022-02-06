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

    template<typename Text>
    constexpr text_cursor(Text const &text, size_t index, bool after) noexcept
    {
        using std::size;

        if (size(text) == 0) {
            index = 0;
            after = false;
        } else if (static_cast<ptrdiff_t>(index) < 0) {
            // Underflow.
            index = 0;
            after = false;
        } else if (index >= size(text)) {
            // Overflow.
            index = size(text) - 1;
            after = true;
        }

        _value = (index << 1) | static_cast<size_t>(after);
    }

    /** Return the neighbor cursor.
     *
     * @param The text size.
     * @return The cursor that is the neighbor of the this cursor.
     *         If this cursor is at start-of-text or end-of-text then this cursor is returned.
     */
    template<typename Text>
    [[nodiscard]] constexpr text_cursor neighbor(Text const &text) const noexcept
    {
        if (before()) {
            return {text, index() - 1, true};
        } else {
            return {text, index() + 1, false};
        }
    }

    template<typename Text>
    [[nodiscard]] constexpr text_cursor after_neighbor(Text const &text) const noexcept
    {
        return before() ? neighbor(text) : *this;
    }

    template<typename Text>
    [[nodiscard]] constexpr text_cursor before_neighbor(Text const &text) const noexcept
    {
        return after() ? neighbor(text) : *this;
    }

    [[nodiscard]] constexpr bool start_of_text() const noexcept
    {
        return _value == 0;
    }

    template<typename Text>
    [[nodiscard]] constexpr bool end_of_text(Text const &text) const noexcept
    {
        using std::size;
        return size(text) == 0 or (index() == size(text) - 1 and after()) or index() >= size(text);
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
