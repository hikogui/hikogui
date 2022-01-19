// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include "../math.hpp"
#include <tuple>
#include <cstdlib>
#include <algorithm>

namespace tt::inline v1{

    class text_selection {
    public:
        constexpr text_selection() noexcept : _cursor(0), _start_first(0), _start_last(0), _finish_first(0), _finish_last(0)
        {
            tt_axiom(holds_invariant());
        }

        constexpr text_selection(text_selection const &) noexcept = default;
        constexpr text_selection(text_selection &&) noexcept = default;
        constexpr text_selection &operator=(text_selection const &) noexcept = default;
        constexpr text_selection &operator=(text_selection &&) noexcept = default;

        constexpr std::size_t cursor() const noexcept
        {
            return _cursor;
        }

        constexpr std::pair<std::size_t,std::size_t> selection() const noexcept
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
            inplace_min(_cursor, text_size);
            _start_first = _start_last = _finish_first = _finish_last = _cursor;
            tt_axiom(holds_invariant());
        }

        constexpr void set_cursor(std::size_t index) noexcept
        {
            _cursor = _start_first = _start_last = _finish_first = _finish_last = index;
            tt_axiom(holds_invariant());
        }

        constexpr void start_selection(std::size_t index, std::size_t first, std::size_t last) noexcept
        {
            _start_first = _finish_first = first;
            _start_last = _finish_last = last;
            _cursor = index == first ? first : last;
            tt_axiom(holds_invariant());
        }

        constexpr void start_selection(std::size_t index, std::pair<std::size_t, std::size_t> selection) noexcept
        {
            return start_selection(index, selection.first, selection.second);
        }

        constexpr void drag_selection(std::size_t index) noexcept
        {
            _finish_first = _finish_last = index;
            _cursor = index;
        }

        constexpr void drag_selection(std::size_t index, std::size_t first, std::size_t last) noexcept
        {
            _finish_first = first;
            _finish_last = last;
            _cursor =
                first < _start_first ? first :
                last > _start_last ? last :
                index == first ? first : last;
            tt_axiom(holds_invariant());
        }

        constexpr void drag_selection(std::size_t index, std::pair<std::size_t, std::size_t> selection) noexcept
        {
            return drag_selection(index, selection.first, selection.second);
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
        std::size_t _cursor;

        /** The first character, at the start of the selection.
         */
        std::size_t _start_first;

        /** One beyond the last character, at the start of the selection.
         */
        std::size_t _start_last;

        /** The first character, at the end of the selection.
         */
        std::size_t _finish_first;

        /** One beyond the last character, at the end of the selection.
         */
        std::size_t _finish_last;
};

}
