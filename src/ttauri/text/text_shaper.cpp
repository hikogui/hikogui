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

static void layout_lines_vertical_spacing(text_shaper::line_vector &lines, float paragraph_spacing, float line_spacing) noexcept
{
    tt_axiom(not lines.empty());

    auto prev = lines.begin();
    auto y = prev->y = 0.0f;
    for (auto it = prev + 1; it != lines.end(); ++it) {
        ttlet height = prev->metrics.descender + std::max(prev->metrics.line_gap, it->metrics.line_gap) + it->metrics.ascender;
        ttlet spacing = prev->end_of_paragraph ? paragraph_spacing : line_spacing;
        // Lines advance downward on the y-axis.
        it->y = y -= spacing * height;
        prev = it;
    }
}

static void layout_lines_vertical_alignment(
    text_shaper::line_vector &lines,
    vertical_alignment alignment,
    float base_line,
    float min_y,
    float max_y,
    float sub_pixel_height) noexcept
{
    tt_axiom(not lines.empty());

    // Calculate the y-adjustment needed to position the base-line of the text to y=0
    auto adjustment = [&]() {
        if (alignment == vertical_alignment::top) {
            return -lines.front().y;

        } else if (alignment == vertical_alignment::bottom) {
            return -lines.back().y;

        } else {
            ttlet mp_index = lines.size() / 2;
            if (lines.size() % 2 == 1) {
                return -lines[mp_index].y;

            } else {
                return -std::midpoint(lines[mp_index - 1].y, lines[mp_index].y);
            }
        }
    }();

    // Add the base-line to the adjustment.
    adjustment += base_line;

    // Clamp the adjustment between min_y and max_y.
    // The text may not fit, prioritize to show the top lines.
    if (lines.back().y + adjustment < min_y) {
        adjustment = min_y - lines.back().y;
    }
    if (lines.front().y + adjustment > max_y) {
        adjustment = max_y - lines.front().y;
    }

    // Reposition the lines, and round to sub-pixel boundary.
    ttlet rcp_sub_pixel_height = 1.0f / sub_pixel_height;
    for (auto &line : lines) {
        line.y = std::round((line.y + adjustment) * rcp_sub_pixel_height) * sub_pixel_height;
    }
}

/** Run the bidi-algorithm over the text and replace the columns of each line.
 *
 * @param[in,out] lines The lines to be modified
 * @param[in,out] text The input text. non-const because modifications on the text is required.
 * @param writing_direction The initial writing direction.
 */
static void
bidi_algorithm(text_shaper::line_vector &lines, text_shaper::char_vector &text, unicode_bidi_class writing_direction) noexcept
{
    tt_axiom(not lines.empty());

    // Create a list of all character indices.
    auto char_its = std::vector<text_shaper::char_iterator>{};
    char_its.reserve(text.size());
    for (auto it = text.begin(); it != text.end(); ++it) {
        char_its.push_back(it);
    }

    // Reorder the character indices based on the unicode bidi algorithm.
    auto context = unicode_bidi_context{};
    context.default_paragraph_direction = writing_direction;
    ttlet[char_its_last, paragraph_directions] = unicode_bidi(
        char_its.begin(),
        char_its.end(),
        [&](text_shaper::char_const_iterator it) {
            return it->grapheme[0];
        },
        [&](text_shaper::char_iterator it, char32_t code_point) {
            it->replace_glyph(code_point);
        },
        [&](text_shaper::char_iterator it, unicode_bidi_class direction) {
            it->direction = direction;
        },
        context);

    // The unicode bidi algorithm may have deleted a few characters.
    char_its.erase(char_its_last, char_its.cend());

    // Add the paragraph direction for each line.
    auto par_it = paragraph_directions.cbegin();
    for (auto &line : lines) {
        tt_axiom(par_it != paragraph_directions.cend());
        line.paragraph_direction = *par_it;
        if (line.end_of_paragraph) {
            par_it++;
        }
    }
    tt_axiom(par_it == paragraph_directions.cend());

    // Add the character indices for each line in display order.
    auto line_it = lines.begin();
    line_it->columns.clear();
    for (ttlet char_it : char_its) {
        if (char_it >= line_it->last) {
            ++line_it;
            line_it->columns.clear();
        }
        tt_axiom(line_it != lines.end());
        tt_axiom(char_it >= line_it->first and char_it < line_it->last);
        line_it->columns.push_back(char_it);
    }
}

[[nodiscard]] text_shaper::text_shaper(tt::font_book &font_book, gstring const &text, text_style const &style) noexcept :
    _font_book(&font_book)
{
    ttlet &font = font_book.find_font(style.family_id, style.variant);
    _text.reserve(text.size());
    for (ttlet &c : text) {
        ttlet clean_c = c == '\n' ? grapheme{paragraph_separator_character} : c;

        auto &tmp = _text.emplace_back(clean_c, style);
        tmp.initialize_glyph(font_book, font);
    }
}

[[nodiscard]] text_shaper::text_shaper(font_book &font_book, std::string_view text, text_style const &style) noexcept :
    text_shaper(font_book, to_gstring(text), style)
{
}

[[nodiscard]] text_shaper::line_vector text_shaper::make_lines(
    aarectangle rectangle,
    float base_line,
    extent2 sub_pixel_size,
    tt::vertical_alignment vertical_alignment = tt::vertical_alignment::middle,
    float line_spacing = 1.0f,
    float paragraph_spacing = 1.5f) const noexcept
{
    ttlet line_sizes = unicode_break_lines(
        _text.begin(),
        _text.end(),
        rectangle.width(),
        [](ttlet &c) {
            return *(c.description);
        },
        [](ttlet &c) {
            return c.width;
        });

    auto r = text_shaper::line_vector{};
    r.reserve(line_sizes.size());

    auto first = _text.begin();
    for (ttlet line_size : line_sizes) {
        tt_axiom(line_size > 0);
        ttlet last = first + line_size;
        r.emplace_back(_text.begin(), first, last);
        first = last;
    }
    if (not r.empty()) {
        r.back().end_of_paragraph = true;
        layout_lines_vertical_spacing(r, line_spacing, paragraph_spacing);
        layout_lines_vertical_alignment(
            r, vertical_alignment, base_line, rectangle.bottom(), rectangle.top(), sub_pixel_size.height());
    }

    return r;
}

void text_shaper::position_glyphs(
    aarectangle rectangle,
    extent2 sub_pixel_size,
    tt::horizontal_alignment horizontal_alignment,
    unicode_bidi_class writing_direction) noexcept
{
    tt_axiom(not _lines.empty());

    // The bidi algorithm will reorder the characters on each line, and mirror the brackets in the text when needed.
    bidi_algorithm(_lines, _text, writing_direction);
    for (auto &line : _lines) {
        // Position the glyphs on each line. Possibly morph glyphs to handle ligatures and calculate the bounding rectangles.
        line.layout(horizontal_alignment, rectangle.left(), rectangle.right(), sub_pixel_size.width());
    }
}

[[nodiscard]] std::pair<aarectangle, float> text_shaper::bounding_rectangle(
    float maximum_line_width,
    tt::vertical_alignment vertical_alignment,
    float line_spacing,
    float paragraph_spacing) const noexcept
{
    ttlet rectangle = aarectangle{
        point2{0.0f, std::numeric_limits<float>::lowest()}, point2{maximum_line_width, std::numeric_limits<float>::max()}};
    constexpr auto base_line = 0.0f;
    constexpr auto sub_pixel_size = extent2{1.0f, 1.0f};

    ttlet lines = make_lines(rectangle, base_line, sub_pixel_size, vertical_alignment, line_spacing, paragraph_spacing);

    if (lines.empty()) {
        return {aarectangle{}, 0.0f};
    }

    auto max_width = 0.0f;
    for (auto &line : lines) {
        inplace_max(max_width, line.width);
    }

    // clang-format off
    ttlet cap_height =
        vertical_alignment == vertical_alignment::bottom ? lines.back().metrics.cap_height :
        vertical_alignment == vertical_alignment::top ? lines.front().metrics.cap_height :
        lines[lines.size() / 2].metrics.cap_height;
    // clang-format on

    ttlet max_y = lines.front().y + std::ceil(lines.front().metrics.x_height);
    ttlet min_y = lines.back().y;
    return {aarectangle{point2{0.0f, min_y}, point2{std::ceil(max_width), max_y}}, cap_height};
}

[[nodiscard]] void text_shaper::layout(
    aarectangle rectangle,
    float base_line,
    extent2 sub_pixel_size,
    unicode_bidi_class writing_direction,
    tt::alignment alignment,
    float line_spacing,
    float paragraph_spacing) noexcept
{
    _lines = make_lines(rectangle, base_line, sub_pixel_size, alignment.vertical(), line_spacing, paragraph_spacing);
    if (not _lines.empty()) {
        position_glyphs(rectangle, sub_pixel_size, alignment.text(), writing_direction);
    }
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
