// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_shaper.hpp"
#include "font_book.hpp"
#include "unicode_line_break.hpp"
#include "unicode_bidi.hpp"
#include "../log.hpp"
#include <numeric>

namespace tt::inline v1 {

[[nodiscard]] text_shaper::char_type::char_type(tt::grapheme const &grapheme, text_style const &style) noexcept :
    grapheme(grapheme), style(style), description(&unicode_description_find(grapheme[0]))
{
}

void text_shaper::char_type::set_glyph(tt::glyph_ids &&new_glyph) noexcept
{
    glyph = std::move(new_glyph);
    auto glyph_metrics = tt::glyph_metrics{};
    if (glyph.font().load_glyph_metrics(glyph[0], glyph_metrics)) {
        scale = glyph.font().metrics.round_scale(style.scaled_size());
        metrics = scale * glyph_metrics;
    } else {
        // Failed to load metrics, due to corrupt font file.
        metrics = {};
    }
}

void text_shaper::char_type::initialize_glyph(tt::font_book &font_book, tt::font const &font) noexcept
{
    if (not glyph_is_initial) {
        set_glyph(font_book.find_glyph(font, grapheme));

        bounding_rectangle = metrics.bounding_rectangle;
        width = metrics.advance.x();
        glyph_is_initial = true;
    }
}

void text_shaper::char_type::initialize_glyph(tt::font_book &font_book) noexcept
{
    return initialize_glyph(font_book, font_book.find_font(style.family_id, style.variant));
}

void text_shaper::char_type::replace_glyph(char32_t code_point) noexcept
{
    ttlet &font = glyph.font();
    set_glyph(font.find_glyph(tt::grapheme{code_point}));

    bounding_rectangle = metrics.bounding_rectangle;
    glyph_is_initial = false;
}

[[nodiscard]] float text_shaper::line_type::calculate_width() noexcept
{
    return unicode_LB_width(
        first,
        last,
        [](ttlet &c) {
            return c.description->general_category();
        },
        [](ttlet &c) {
            return c.width;
        });
}

text_shaper::line_type::line_type(
    text_shaper::char_const_iterator begin,
    text_shaper::char_const_iterator first,
    text_shaper::char_const_iterator last) noexcept :
    first(first), last(last), columns(), metrics(), y(0.0f), width(0.0f), end_of_paragraph(false)
{
    tt_axiom(first != last);

    width = calculate_width();

    for (auto it = first; it != last; ++it) {
        metrics = max(metrics, it->font_metrics());
    }

    ttlet last_category = (last - 1)->description->general_category();
    if (last_category == unicode_general_category::Zp) {
        end_of_paragraph = true;
    }
}

[[nodiscard]] text_shaper::text_shaper(tt::font_book &font_book, gstring const &text, text_style const &style) noexcept :
    _font_book(&font_book)
{
    ttlet &font = font_book.find_font(style.family_id, style.variant);
    _text.reserve(text.size());
    for (ttlet &c : text) {
        auto &tmp = _text.emplace_back(c, style);
        tmp.initialize_glyph(font_book, font);
    }
}

[[nodiscard]] text_shaper::text_shaper(font_book &font_book, std::string_view text, text_style const &style) noexcept :
    text_shaper(font_book, to_gstring(text), style)
{
}

[[nodiscard]] std::vector<size_t> text_shaper::fold_lines(float maximum_line_width) const noexcept
{
    return unicode_break_lines(
        _text.begin(),
        _text.end(),
        maximum_line_width,
        [](ttlet &c) {
            return *(c.description);
        },
        [](ttlet &c) {
            return c.width;
        });
}

[[nodiscard]] std::vector<text_shaper::line_type> text_shaper::layout_create_lines(float maximum_line_width) const noexcept
{
    ttlet line_sizes = fold_lines(maximum_line_width);

    auto r = std::vector<text_shaper::line_type>{};
    r.reserve(line_sizes.size());

    auto first = _text.begin();
    for (ttlet line_size : line_sizes) {
        tt_axiom(line_size > 0);
        ttlet last = first + line_size;
        r.emplace_back(_text.begin(), first, last);
        first = last;
    }
    return r;
}

void text_shaper::layout_lines_vertical_spacing(float paragraph_spacing, float line_spacing) noexcept
{
    if (_lines.empty()) {
        return;
    }

    auto prev = _lines.begin();
    auto y = prev->y = 0.0f;
    for (auto it = prev + 1; it != _lines.end(); ++it) {
        ttlet height = prev->metrics.descender + std::max(prev->metrics.line_gap, it->metrics.line_gap) + it->metrics.ascender;
        ttlet spacing = prev->end_of_paragraph ? paragraph_spacing : line_spacing;
        // Lines advance downward on the y-axis.
        it->y = y -= spacing * height;
        prev = it;
    }
}

[[nodiscard]] float text_shaper::layout_lines_vertical_adjustment(vertical_alignment alignment) const noexcept
{
    if (_lines.empty()) {
        return -0.0f;
    }

    if (alignment == vertical_alignment::top) {
        return -_lines.front().y;
    } else if (alignment == vertical_alignment::bottom) {
        return -_lines.back().y;
    } else {
        auto mp_index = _lines.size() / 2;
        if (_lines.size() % 2 == 1) {
            return -_lines[mp_index].y;

        } else {
            return -std::midpoint(_lines[mp_index - 1].y, _lines[mp_index].y);
        }
    }
}

[[nodiscard]] aarectangle text_shaper::layout_lines(
    float maximum_line_width,
    vertical_alignment alignment,
    float line_spacing,
    float paragraph_spacing) noexcept
{
    _lines = layout_create_lines(maximum_line_width);
    layout_lines_vertical_spacing(line_spacing, paragraph_spacing);
    ttlet adjustment = layout_lines_vertical_adjustment(alignment);

    auto max_width = 0.0f;
    for (auto &line : _lines) {
        line.y += adjustment;
        inplace_max(max_width, line.width);
    }

    if (_lines.empty()) {
        return aarectangle{};
    } else {
        ttlet max_y = std::ceil(_lines.front().y + _lines.front().metrics.x_height);
        ttlet min_y = std::floor(_lines.back().y);
        return aarectangle{point2{0.0f, min_y}, point2{std::ceil(max_width), max_y}};
    }
}

void text_shaper::reorder_glyphs(unicode_bidi_class writing_direction) noexcept
{
    // Create a list of all character indices.
    auto char_its = std::vector<char_iterator>{};
    char_its.reserve(_text.size());
    for (auto it = _text.begin(); it != _text.end(); ++it) {
        char_its.push_back(it);
    }

    // Reorder the character indices based on the unicode bidi algorithm.
    auto context = unicode_bidi_context{};
    context.default_paragraph_direction = writing_direction;
    ttlet[char_its_last, paragraph_directions] = unicode_bidi(
        char_its.begin(),
        char_its.end(),
        [&](char_const_iterator it) {
            return it->grapheme[0];
        },
        [&](char_iterator it, char32_t code_point) {
            it->replace_glyph(code_point);
        },
        [&](ttlet...) {},
        context);

    // The unicode bidi algorithm may have deleted a few characters.
    char_its.erase(char_its_last, char_its.cend());

    // Add the paragraph direction for each line.
    tt_axiom(not _lines.empty());
    auto par_it = paragraph_directions.cbegin();
    for (auto &line : _lines) {
        tt_axiom(par_it != paragraph_directions.cend());
        line.paragraph_direction = *par_it;
        if (line.end_of_paragraph) {
            par_it++;
        }
    }

    // Add the character indices for each line in display order.
    tt_axiom(not _lines.empty());
    auto line_it = _lines.begin();
    line_it->columns.clear();
    for (ttlet char_it : char_its) {
        if (char_it >= line_it->last) {
            ++line_it;
            line_it->columns.clear();
        }
        tt_axiom(line_it != _lines.end());
        tt_axiom(char_it >= line_it->first and char_it < line_it->last);
        line_it->columns.push_back(char_it);
    }
}

void text_shaper::resolve_text_alignment(text_alignment alignment) noexcept
{
    tt_axiom(not _lines.empty());
    for (auto &line : _lines) {
        if (alignment == text_alignment::flush) {
            if (line.paragraph_direction == unicode_bidi_class::L) {
                line.paragraph_alignment = text_alignment::flush_left;
            } else if (line.paragraph_direction == unicode_bidi_class::R) {
                line.paragraph_alignment = text_alignment::flush_right;
            } else {
                tt_no_default();
            }
        } else {
            line.paragraph_alignment = alignment;
        }
    }
}

void text_shaper::position_glyphs(
    aarectangle rectangle,
    float base_line,
    text_alignment alignment,
    unicode_bidi_class writing_direction,
    extent2 sub_pixel_size) noexcept
{
    reorder_glyphs(writing_direction);
    resolve_text_alignment(alignment);
    // morph_glyphs();
    // kern_glyphs();
    // reposition_lines(rectangle, base_line);
    // advance_glyphs(sub_pixel_size);
}

//[[nodiscard]] size_t text_shaper::get_char(size_t column_nr, size_t line_nr) const noexcept
//{
//    return _line[line_nr].column[column_nr];
//}
//
//[[nodiscard]] std::pair<size_t, size_t> text_shaper::get_column_line(size_t index) const noexcept
//{
//    tt_axiom(index < _chars.size());
//
//    for (auto line_nr = 0_uz; line_nr != _lines.size(); ++line_nr) {
//        ttlet &line = _lines[line_nr];
//        for (auto column_nr = 0_uz; column_nr != line.columns.size(); ++column_nr) {
//            if (line.columns[column_nr] == index) {
//                return {column_nr, line_nr};
//            }
//        }
//    }
//    tt_no_default();
//}
//
//[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_left(size_t column_nr, size_t line_nr) const noexcept
//{
//    if (column_nr >= 0) {
//        --column_nr;
//    } else if (line_nr >= 0) {
//        --line_nr;
//        tt_axiom(not _lines[line_nr].columns.empty());
//        column_nr = _lines[line_nr].columns.size() - 1;
//    }
//    return {column_nr, line_nr};
//}
//
//[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_right(size_t column_nr, size_t line_nr) const noexcept
//{
//    if (column_nr < _lines[line_nr].columns.size()) {
//        ++column_nr;
//    } else if (line_nr < _lines.size()) {
//        ++line_nr;
//        tt_axiom(not _lines[line_nr].columns.empty());
//        column_nr = 0;
//    }
//    return {column_nr, line_nr};
//}
//
//[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_up(size_t column_nr, size_t line_nr) const noexcept
//{
//    if (line_nr >= 0) {
//        --line_nr;
//        tt_axiom(not _lines[line_nr].columns.empty());
//        inplace_min(column_nr, _lines[line_nr].columns.size() - 1);
//    }
//    return {column_nr, line_nr};
//}
//
//[[nodiscard]] std::pair<size_t, size_t> text_shaper::go_down(size_t column_nr, size_t line_nr) const noexcept
//{
//    if (line_nr < _lines.size()) {
//        ++line_nr;
//        tt_axiom(not _lines[line_nr].columns.empty());
//        inplace_min(column_nr, _lines[line_nr].columns.size() - 1);
//    }
//    return {column_nr, line_nr};
//}
//
//[[nodiscard]] size_t text_shaper::char_left_of(size_t index) const noexcept
//{
//    ttlet[column_nr, line_nr] = get_column_line(index);
//    ttlet[new_column_nr, new_line_nr] = go_left(column_nr, line_nr);
//    return get_char(new_column_nr, new_line_nr);
//}
//
//[[nodiscard]] ssize_t text_shaper::right_of(ssize_t index) const noexcept
//{
//    ttlet[column_nr, line_nr] = get_column_line(index);
//    ttlet[new_column_nr, new_line_nr] = go_right(column_nr, line_nr);
//    return get_char(new_column_nr, new_line_nr);
//}
//
//[[nodiscard]] ssize_t text_shaper::above(ssize_t index) const noexcept
//{
//    ttlet[column_nr, line_nr] = get_column_line(index);
//    ttlet[new_column_nr, new_line_nr] = go_up(column_nr, line_nr);
//    return get_char(new_column_nr, new_line_nr);
//}
//
//[[nodiscard]] ssize_t text_shaper::below(ssize_t index) const noexcept
//{
//    ttlet[column_nr, line_nr] = get_column_line(index);
//    ttlet[new_column_nr, new_line_nr] = go_down(column_nr, line_nr);
//    return get_char(new_column_nr, new_line_nr);
//}

} // namespace tt::inline v1
