// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_cursor.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <tuple>
#include <cstdlib>
#include <algorithm>

hi_export_module(hikogui.text.text_selection);


hi_export namespace hi::inline v1 {

class text_selection {
public:
    constexpr text_selection() noexcept = default;
    constexpr text_selection(text_selection const&) noexcept = default;
    constexpr text_selection(text_selection&&) noexcept = default;
    constexpr text_selection& operator=(text_selection const&) noexcept = default;
    constexpr text_selection& operator=(text_selection&&) noexcept = default;

    constexpr text_selection& resize(size_t size) noexcept
    {
        _cursor.resize(size);
        _start_first.resize(size);
        _start_last.resize(size);
        _finish_first.resize(size);
        _finish_last.resize(size);
        return *this;
    }

    constexpr text_cursor cursor() const noexcept
    {
        return _cursor;
    }

    /** Return the selection of characters.
     *
     * @param size The size of the text.
     * @return Cursor before the first character, Cursor after the last character.
     */
    constexpr std::pair<text_cursor, text_cursor> selection(size_t size) const noexcept
    {
        auto first = std::min(_start_first, _finish_first);
        auto last = std::max(_start_last, _finish_last);

        first = first.before_neighbor(size);
        last = last.after_neighbor(size);
        return {first, last};
    }

    /** Get the text indices for the selection.
     *
     */
    constexpr std::pair<size_t, size_t> selection_indices() const noexcept
    {
        auto first = std::min(_start_first, _finish_first);
        auto last = std::max(_start_last, _finish_last);

        auto first_index = first.after() ? first.index() + 1 : first.index();
        auto last_index = last.after() ? last.index() + 1 : last.index();
        return {first_index, last_index};
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        hilet[first_index, last_index] = selection_indices();
        return first_index >= last_index;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    constexpr text_selection& clear_selection(size_t size) noexcept
    {
        return set_cursor(_cursor.resize(size));
    }

    constexpr text_selection& set_cursor(text_cursor new_cursor) noexcept
    {
        _cursor = _start_first = _start_last = _finish_first = _finish_last = new_cursor;
        hi_axiom(holds_invariant());
        return *this;
    }

    constexpr text_selection& operator=(text_cursor const& rhs) noexcept
    {
        return set_cursor(rhs);
    }

    constexpr void start_selection(text_cursor new_cursor, text_cursor first, text_cursor last) noexcept
    {
        _start_first = _finish_first = first;
        _start_last = _finish_last = last;
        _cursor = new_cursor == first ? first : last;
        hi_axiom(holds_invariant());
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
        _cursor = first < _start_first ? first : last > _start_last ? last : drag_cursor == first ? first : last;
        hi_axiom(holds_invariant());
    }

    constexpr void drag_selection(text_cursor drag_cursor, std::pair<text_cursor, text_cursor> selection) noexcept
    {
        return drag_selection(drag_cursor, selection.first, selection.second);
    }

    [[nodiscard]] constexpr friend bool operator==(text_selection const&, text_selection const&) noexcept = default;

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

} // namespace hi::inline v1
