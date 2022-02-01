// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_shaper.hpp"
#include "font_book.hpp"
#include "../unicode/unicode_line_break.hpp"
#include "../unicode/unicode_bidi.hpp"
#include "../unicode/unicode_word_break.hpp"
#include "../unicode/unicode_sentence_break.hpp"
#include "../log.hpp"
#include <numeric>
#include <ranges>
#include <algorithm>

namespace tt::inline v1 {

static void layout_lines_vertical_spacing(text_shaper::line_vector &lines, float line_spacing, float paragraph_spacing) noexcept
{
    tt_axiom(not lines.empty());

    auto prev = lines.begin();
    prev->y = 0.0f;
    for (auto it = prev + 1; it != lines.end(); ++it) {
        ttlet height = prev->metrics.descender + std::max(prev->metrics.line_gap, it->metrics.line_gap) + it->metrics.ascender;
        ttlet spacing = prev->last_category == unicode_general_category::Zp ? paragraph_spacing : line_spacing;
        // Lines advance downward on the y-axis.
        it->y = prev->y - spacing * height;
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
    // Make room for implicit line-separators.
    char_its.reserve(text.size() + lines.size());
    for (ttlet &line : lines) {
        // Add all the characters of a line.
        for (auto it = line.first; it != line.last; ++it) {
            char_its.push_back(it);
        }
        if (line.last_category != unicode_general_category::Zp and line.last_category != unicode_general_category::Zl) {
            // No explicit paragraph-separator or line-separator, at a virtual one.
            char_its.push_back(text.end());
        }
    }

    // Reorder the character indices based on the unicode bidi algorithm.
    auto context = unicode_bidi_context{};

    if (writing_direction == unicode_bidi_class::L) {
        context.direction_mode = unicode_bidi_context::mode_type::auto_LTR;
    } else if (writing_direction == unicode_bidi_class::R) {
        context.direction_mode = unicode_bidi_context::mode_type::auto_RTL;
    } else {
        tt_no_default();
    }

    ttlet[char_its_last, paragraph_directions] = unicode_bidi(
        char_its.begin(),
        char_its.end(),
        [&](text_shaper::char_const_iterator it) -> decltype(auto) {
            if (it != text.end()) {
                return *it->description;
            } else {
                return unicode_description_find(unicode_LS);
            }
        },
        [&](text_shaper::char_iterator it, char32_t code_point) {
            tt_axiom(it != text.end());
            it->replace_glyph(code_point);
        },
        [&](text_shaper::char_iterator it, unicode_bidi_class direction) {
            if (it != text.end()) {
                it->direction = direction;
            }
        },
        context);

    // The unicode bidi algorithm may have deleted a few characters.
    char_its.erase(char_its_last, char_its.cend());

    // Add the paragraph direction for each line.
    auto par_it = paragraph_directions.cbegin();
    for (auto &line : lines) {
        tt_axiom(par_it != paragraph_directions.cend());
        line.paragraph_direction = *par_it;
        if (line.last_category == unicode_general_category::Zp) {
            par_it++;
        }
    }
    tt_axiom(par_it <= paragraph_directions.cend());

    // Add the character indices for each line in display order.
    auto line_it = lines.begin();
    line_it->columns.clear();
    auto column_nr = 0_uz;
    for (ttlet char_it : char_its) {
        if (char_it == text.end()) {
            // Ignore the virtual line separators.
            continue;
        } else if (char_it >= line_it->last) {
            // Skip to the next line.
            tt_axiom(line_it->columns.size() <= narrow_cast<size_t>(std::distance(line_it->first, line_it->last)));
            ++line_it;
            line_it->columns.clear();
            column_nr = 0_uz;
        }
        tt_axiom(line_it != lines.end());
        tt_axiom(char_it >= line_it->first);
        tt_axiom(char_it < line_it->last);
        line_it->columns.push_back(char_it);

        // Assign line_nr and column_nr, for quick back referencing.
        char_it->line_nr = line_it->line_nr;
        char_it->column_nr = column_nr++;
    }
}

[[nodiscard]] text_shaper::text_shaper(tt::font_book &font_book, gstring const &text, text_style const &style) noexcept :
    _font_book(&font_book)
{
    ttlet &font = font_book.find_font(style.family_id, style.variant);
    _text.reserve(text.size());
    for (ttlet &c : text) {
        ttlet clean_c = c == '\n' ? grapheme{unicode_PS} : c;

        auto &tmp = _text.emplace_back(clean_c, style);
        tmp.initialize_glyph(font_book, font);
    }

    _line_break_opportunities = unicode_line_break(_text.begin(), _text.end(), [](ttlet &c) -> decltype(auto) {
        tt_axiom(c.description != nullptr);
        return *c.description;
    });

    _line_break_widths.reserve(text.size());
    for (ttlet &c : _text) {
        _line_break_widths.push_back(is_visible(c.description->general_category()) ? c.width : -c.width);
    }

    _word_break_opportunities = unicode_word_break(_text.begin(), _text.end(), [](ttlet &c) -> decltype(auto) {
        tt_axiom(c.description != nullptr);
        return *c.description;
    });

    _sentence_break_opportunities = unicode_sentence_break(_text.begin(), _text.end(), [](ttlet &c) -> decltype(auto) {
        tt_axiom(c.description != nullptr);
        return *c.description;
    });
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
    float paragraph_spacing = 1.5f) noexcept
{
    ttlet line_sizes = unicode_line_break(_line_break_opportunities, _line_break_widths, rectangle.width());

    auto r = text_shaper::line_vector{};
    r.reserve(line_sizes.size());

    auto char_it = _text.begin();
    auto width_it = _line_break_widths.begin();
    auto line_nr = 0_uz;
    for (ttlet line_size : line_sizes) {
        tt_axiom(line_size > 0);
        ttlet char_eol = char_it + line_size;
        ttlet width_eol = width_it + line_size;

        ttlet line_width = unicode_line_break_width(width_it, width_eol);
        r.emplace_back(line_nr++, _text.begin(), char_it, char_eol, line_width);

        char_it = char_eol;
        width_it = width_eol;
    }

    if (not r.empty()) {
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
    float paragraph_spacing) noexcept
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

    ttlet max_y = lines.front().y + std::ceil(lines.front().metrics.ascender);
    ttlet min_y = lines.back().y - std::ceil(lines.back().metrics.descender);
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

[[nodiscard]] text_cursor text_shaper::get_nearest(point2 position) const noexcept
{
    ttlet line_it = std::ranges::min_element(_lines, std::ranges::less{}, [position](ttlet &line) {
        return distance(line.rectangle, position);
    });

    if (line_it != _lines.end()) {
        ttlet[char_it, after] = line_it->get_nearest(position);
        return {narrow<size_t>(std::distance(_text.begin(), char_it)), after};
    } else {
        return {};
    }
}

[[nodiscard]] static std::pair<text_cursor, text_cursor>
get_selection_from_break(text_cursor cursor, unicode_break_vector const &break_opportunities) noexcept
{
    // In the algorithm below we search before and after the character that the cursor is at.
    // We do not use the before/after differentiation.

    ttlet first_index = [&]() {
        auto i = cursor.index();
        while (break_opportunities[i] == unicode_break_opportunity::no) {
            --i;
        }
        return i;
    }();
    ttlet last_index = [&]() {
        auto i = cursor.index();
        while (break_opportunities[i + 1] == unicode_break_opportunity::no) {
            ++i;
        }
        return i;
    }();

    return {{first_index, false}, {last_index, true}};
}

[[nodiscard]] std::pair<text_cursor, text_cursor> text_shaper::get_word(text_cursor cursor) const noexcept
{
    return get_selection_from_break(cursor, _word_break_opportunities);
}

[[nodiscard]] std::pair<text_cursor, text_cursor> text_shaper::get_sentence(text_cursor cursor) const noexcept
{
    return get_selection_from_break(cursor, _sentence_break_opportunities);
}

[[nodiscard]] std::optional<text_shaper::char_const_iterator>
text_shaper::left_of(text_shaper::char_const_iterator it) const noexcept
{
    tt_axiom(it->line_nr < _lines.size());
    tt_axiom(it->column_nr < _lines[it->line_nr].columns.size());
    if (it->column_nr == 0) {
        return {};
    }

    return _lines[it->line_nr].columns[it->column_nr - 1];
}

[[nodiscard]] std::optional<text_shaper::char_const_iterator>
text_shaper::right_of(text_shaper::char_const_iterator it) const noexcept
{
    tt_axiom(it->line_nr < _lines.size());
    tt_axiom(it->column_nr < _lines[it->line_nr].columns.size());
    if (it->column_nr == _lines[it->line_nr].columns.size() - 1) {
        return {};
    }

    return _lines[it->line_nr].columns[it->column_nr + 1];
}

} // namespace tt::inline v1
