// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include "../math.hpp"
#include "../cast.hpp"
#include <tuple>
#include <cstdlib>
#include <algorithm>

namespace tt::inline v1{

    class text_cursor {
    public:
        constexpr text_cursor() noexcept : _value(0) {}
        constexpr text_cursor(text_cursor const &) noexcept = default;
        constexpr text_cursor(text_cursor &&) noexcept = default;
        constexpr text_cursor &operator=(text_cursor const &) noexcept = default;
        constexpr text_cursor &operator=(text_cursor &&) noexcept = default;

        constexpr text_cursor(size_t index, bool after) noexcept : _value(index << 1 | static_cast<size_t>(after)) {}

        [[nodiscard]] constexpr text_cursor neighbour() const noexcept
        {
            text_cursor r;
            r._value = _value + (after() ? 1 : -1);
            return r;
        }

        /** Advance the cursor by num_characters.
        * 
        * @param num_characters
        * @return A cursor after the added characters. Or start-of-text cursor.
        */
        [[nodiscard]] constexpr text_cursor advance_char(ptrdiff_t num_characters, size_t text_size) const noexcept
        {
            auto new_index = narrow<ptrdiff_t>(index()) + num_characters;

            if (new_index < 0 or (new_index == 0 and before())) {
                return {0, false};
            } else if (new_index >= narrow<ptrdiff_t>(text_size) or (new_index == narrow<ptrdiff_t>(text_size) - 1 and after())) {
                return {text_size - 1, true};
            } else if (before()) {
                return text_cursor{narrow<size_t>(new_index), after()}.neighbour();
            } else {
                return {narrow<size_t>(new_index), after()};
            }
        }

        [[nodiscard]] constexpr bool start_of_text() const noexcept
        {
            return _value == 0;
        }

        [[nodiscard]] constexpr bool end_of_text(size_t text_size) const noexcept
        {
            auto end_cursor = text_size == 0 ? text_cursor{0, false} : text_cursor{text_size - 1, true};
            tt_axiom(_value <= end_cursor._value);
            return _value == end_cursor._value;
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

}
