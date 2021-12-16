// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_shaper.hpp"

namespace tt::inline v1 {

[[nodiscard]] size_t text_shaper::get_char(size_t column_nr, size_t line_nr) const noexcept
{
    return _line[line_nr].column[column_nr];
}

[[nodiscard]] std::pair<size_t,size_t> text_shaper::get_column_line(size_t index) const noexcept
{
    tt_axiom(index < _chars.size());

    for (auto line_nr = 0_uz; line_nr != _lines.size(); ++line_nr) {
        ttlet &line = _lines[line_nr];
        for (auto column_nr = 0_uz; column_nr != line.columns.size(); ++column_nr) {
            if (line.columns[column_nr] == index) {
                return {column_nr, line_nr};
            }
        }
    }
    tt_no_default();
}

[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_left(size_t column_nr, size_t line_nr) const noexcept
{
    if (column_nr >= 0) {
        --column_nr;
    } else if (line_nr >= 0) {
        --line_nr;
        tt_axiom(not _lines[line_nr].columns.empty());
        column_nr = _lines[line_nr].columns.size() - 1;
    }
    return {column_nr, line_nr};
}

[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_right(size_t column_nr, size_t line_nr) const noexcept
{
    if (column_nr < _lines[line_nr].columns.size()) {
        ++column_nr;
    } else if (line_nr < _lines.size()) {
        ++line_nr;
        tt_axiom(not _lines[line_nr].columns.empty());
        column_nr = 0;
    }
    return {column_nr, line_nr};
}

[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_up(size_t column_nr, size_t line_nr) const noexcept
{
    if (line_nr >= 0) {
        --line_nr;
        tt_axiom(not _lines[line_nr].columns.empty());
        inplace_min(column_nr, _lines[line_nr].columns.size()- 1);
    } 
    return {column_nr, line_nr};
}

[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_down(size_t column_nr, size_t line_nr) const noexcept
{
    if (line_nr < _lines.size()) {
        ++line_nr;
        tt_axiom(not _lines[line_nr].columns.empty());
        inplace_min(column_nr, _lines[line_nr].columns.size()- 1);
    } 
    return {column_nr, line_nr};
}

[[nodiscard]] size_t text_shaper::char_left_of(size_t index) const noexcept
{
    ttlet [column_nr, line_nr] = get_column_line(index);
    ttlet [new_column_nr, new_line_nr] = go_left(column_nr, line_nr);
    return get_char(new_column_nr, new_line_nr);
}

[[nodiscard]] ssize_t text_shaper::right_of(ssize_t index) const noexcept
{
    ttlet [column_nr, line_nr] = get_column_line(index);
    ttlet [new_column_nr, new_line_nr] = go_right(column_nr, line_nr);
    return get_char(new_column_nr, new_line_nr);
}

[[nodiscard]] ssize_t text_shaper::above(ssize_t index) const noexcept
{
    ttlet [column_nr, line_nr] = get_column_line(index);
    ttlet [new_column_nr, new_line_nr] = go_up(column_nr, line_nr);
    return get_char(new_column_nr, new_line_nr);
}

[[nodiscard]] ssize_t text_shaper::below(ssize_t index) const noexcept
{
    ttlet [column_nr, line_nr] = get_column_line(index);
    ttlet [new_column_nr, new_line_nr] = go_down(column_nr, line_nr);
    return get_char(new_column_nr, new_line_nr);
}


}

