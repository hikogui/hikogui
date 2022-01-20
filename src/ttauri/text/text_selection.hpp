// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_cursor.hpp"
#include "../required.hpp"
#include "../assert.hpp"
#include "../math.hpp"
#include <tuple>
#include <cstdlib>
#include <algorithm>

namespace tt::inline v1{

    class text_selection {
    public:
        constexpr text_selection() noexcept : _cursor(), _start_first(), _start_last(), _finish_first(), _finish_last()
        {
            tt_axiom(holds_invariant());
        }

        constexpr text_selection(text_selection const &) noexcept = default;
        constexpr text_selection(text_selection &&) noexcept = default;
        constexpr text_selection &operator=(text_selection const &) noexcept = default;
        constexpr text_selection &operator=(text_selection &&) noexcept = default;

        constexpr text_cursor cursor() const noexcept
        {
            return _cursor;
        }

        constexpr std::pair<text_cursor, text_cursor> selection() const noexcept
        {
            return {std::min(_start_first, _finish_first), std::max(_start_last, _finish_last)};
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            ttlet [first, last] = selection();
            return first == last;
        }

        constexpr operator bool() const noexcept
        {
            return not empty();
        }

        constexpr void clear_selection(size_t text_size = 0) noexcept
        {
            ttlet new_cursor = std::min(_cursor, text_cursor{text_size, false});
            return set_cursor(new_cursor);
        }

        constexpr void set_cursor(text_cursor new_cursor) noexcept
        {
            _cursor = _start_first = _start_last = _finish_first = _finish_last = new_cursor;
            tt_axiom(holds_invariant());
        }

        constexpr void start_selection(text_cursor new_cursor, text_cursor first, text_cursor last) noexcept
        {
            _start_first = _finish_first = first;
            _start_last = _finish_last = last;
            _cursor = new_cursor == first ? first : last;
            tt_axiom(holds_invariant());
        }

        constexpr void start_selection(text_cursor new_cursor, std::pair<text_cursor, text_cursor> selection) noexcept
        {
            return start_selection(new_cursor, selection.first, selection.second);
        }

        constexpr void drag_selection(text_cursor drag_cursor) noexcept
        {
            _finish_first = _finish_last = drag_cursor;
            _cursor = drag_cursor;
        }

        constexpr void drag_selection(text_cursor drag_cursor, text_cursor first, text_cursor last) noexcept
        {
            _finish_first = first;
            _finish_last = last;
            _cursor =
                first < _start_first ? first :
                last > _start_last ? last :
                drag_cursor == first ? first : last;
            tt_axiom(holds_invariant());
        }

        constexpr void drag_selection(text_cursor drag_cursor, std::pair<text_cursor, text_cursor> selection) noexcept
        {
            return drag_selection(drag_cursor, selection.first, selection.second);
        }

        [[nodiscard]] constexpr friend bool operator==(text_selection const &, text_selection const &) noexcept = default;

    private:
        [[nodiscard]] constexpr bool holds_invariant() const noexcept
        {
            return _start_first <= _start_last and _finish_first <= _finish_last and
                (_cursor == _start_first or _cursor == _start_last or _cursor == _finish_first or _cursor == _finish_last);
        }

        /** The character where the cursor position.
        *
        * If the cursor is beyond the end of the text,
        * than cursor is set to the index beyond the end.
         */
        text_cursor _cursor;

        /** The first character, at the start of the selection.
         */
        text_cursor _start_first;

        /** One beyond the last character, at the start of the selection.
         */
        text_cursor _start_last;

        /** The first character, at the end of the selection.
         */
        text_cursor _finish_first;

        /** One beyond the last character, at the end of the selection.
         */
        text_cursor _finish_last;
};

}
