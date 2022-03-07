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
#include <cmath>

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
        if (not is_Zp_or_Zl(line.last_category)) {
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
                return unicode_description::find(unicode_LS);
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

    // All of the characters in the text must be positioned.
    for (auto &c : text) {
        tt_axiom(c.line_nr != std::numeric_limits<size_t>::max() and c.column_nr != std::numeric_limits<size_t>::max());
    }
}

[[nodiscard]] text_shaper::text_shaper(
    tt::font_book &font_book,
    gstring const &text,
    text_style const &style,
    float dpi_scale,
    unicode_script script) noexcept :
    _font_book(&font_book), _dpi_scale(dpi_scale), _script(script)
{
    ttlet &font = font_book.find_font(style.family_id, style.variant);
    _initial_line_metrics = (style.size * dpi_scale) * font.metrics;

    _text.reserve(text.size());
    for (ttlet &c : text) {
        ttlet clean_c = c == '\n' ? grapheme{unicode_PS} : c;

        auto &tmp = _text.emplace_back(clean_c, style, dpi_scale);
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

    resolve_script();
}

[[nodiscard]] text_shaper::text_shaper(
    font_book &font_book,
    std::string_view text,
    text_style const &style,
    float dpi_scale,
    unicode_script script) noexcept :
    text_shaper(font_book, to_gstring(text), style, dpi_scale, script)
{
}

[[nodiscard]] text_shaper::line_vector text_shaper::make_lines(
    aarectangle rectangle,
    float base_line,
    extent2 sub_pixel_size,
    tt::vertical_alignment vertical_alignment,
    unicode_bidi_class writing_direction,
    float line_spacing,
    float paragraph_spacing) noexcept
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
        r.emplace_back(line_nr++, _text.begin(), char_it, char_eol, line_width, _initial_line_metrics);

        char_it = char_eol;
        width_it = width_eol;
    }

    if (r.empty() or is_Zp_or_Zl(r.back().last_category)) {
        r.emplace_back(line_nr++, _text.begin(), _text.end(), _text.end(), 0.0f, _initial_line_metrics);
        r.back().paragraph_direction = writing_direction;
    }

    layout_lines_vertical_spacing(r, line_spacing, paragraph_spacing);
    layout_lines_vertical_alignment(
        r, vertical_alignment, base_line, rectangle.bottom(), rectangle.top(), sub_pixel_size.height());

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

void text_shaper::resolve_script() noexcept
{
    // Find the first script in the text if no script is found use the text_shaper's default script.
    auto first_script = _script;
    for (auto &c : _text) {
        ttlet script = c.description->script();
        if (script != unicode_script::Common or script == unicode_script::Unknown or script == unicode_script::Inherited) {
            first_script = script;
            break;
        }
    }

    // Backward pass: fix start of words and open-brackets.
    // After this pass unknown-script is no longer in the text.
    // Close brackets will not be fixed, those will be fixed in the last forward pass.
    auto word_script = unicode_script::Common;
    auto previous_script = first_script;
    for (auto i = std::ssize(_text) - 1; i >= 0; --i) {
        auto &c = _text[i];

        if (_word_break_opportunities[i + 1] != unicode_break_opportunity::no) {
            word_script = unicode_script::Common;
        }

        c.script = c.description->script();
        if (c.script == unicode_script::Common or c.script == unicode_script::Unknown) {
            ttlet bracket_type = c.description->bidi_bracket_type();
            // clang-format off
            c.script =
                bracket_type == unicode_bidi_bracket_type::o ? previous_script :
                bracket_type == unicode_bidi_bracket_type::c ? unicode_script::Common :
                word_script;
            // clang-format on

        } else if (c.script != unicode_script::Inherited) {
            previous_script = word_script = c.script;
        }
    }

    // Forward pass: fix all common and inherited with previous or first script.
    previous_script = first_script;
    for (auto i = 0_uz; i != _text.size(); ++i) {
        auto &c = _text[i];

        if (c.script == unicode_script::Common or c.script == unicode_script::Inherited) {
            c.script = previous_script;

        } else {
            previous_script = c.script;
        }
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

    ttlet lines = make_lines(
        rectangle, base_line, sub_pixel_size, vertical_alignment, unicode_bidi_class::L, line_spacing, paragraph_spacing);
    tt_axiom(not lines.empty());

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
    _rectangle = rectangle;
    _lines = make_lines(
        rectangle, base_line, sub_pixel_size, alignment.vertical(), writing_direction, line_spacing, paragraph_spacing);
    tt_axiom(not _lines.empty());
    position_glyphs(rectangle, sub_pixel_size, alignment.text(), writing_direction);
}

[[nodiscard]] text_shaper::char_const_iterator text_shaper::get_it(size_t index) const noexcept
{
    if (static_cast<ptrdiff_t>(index) < 0) {
        return begin();
    } else if (index >= size()) {
        return end();
    }

    return begin() + index;
}

[[nodiscard]] text_shaper::char_const_iterator text_shaper::get_it(size_t column_nr, size_t line_nr) const noexcept
{
    tt_axiom(not _lines.empty());

    if (static_cast<ptrdiff_t>(line_nr) < 0) {
        return begin();
    } else if (line_nr >= _lines.size()) {
        return end();
    }

    ttlet left_of_line = static_cast<ptrdiff_t>(column_nr) < 0;
    ttlet right_of_line = column_nr >= _lines[line_nr].size();

    if (left_of_line or right_of_line) {
        ttlet ltr = _lines[line_nr].paragraph_direction == unicode_bidi_class::L;
        ttlet go_up = left_of_line == ltr;
        if (go_up) {
            // Go to line above.
            if (static_cast<ptrdiff_t>(--line_nr) < 0) {
                return begin();
            } else {
                // Go to end of line above.
                return _lines[line_nr].paragraph_direction == unicode_bidi_class::L ? _lines[line_nr].back() :
                                                                                      _lines[line_nr].front();
            }

        } else {
            // Go to the line below.
            if (++line_nr >= _lines.size()) {
                return end();
            } else {
                // Go to begin of line below.
                return _lines[line_nr].paragraph_direction == unicode_bidi_class::L ? _lines[line_nr].front() :
                                                                                      _lines[line_nr].back();
            }
        }
    }

    return _lines[line_nr][column_nr];
}

[[nodiscard]] std::pair<size_t, size_t> text_shaper::get_column_line(text_shaper::char_const_iterator it) const noexcept
{
    if (it != end()) {
        return {it->column_nr, it->line_nr};
    } else {
        tt_axiom(not _lines.empty());
        return {_lines.size() - 1, _lines.back().size()};
    }
}

[[nodiscard]] size_t text_shaper::get_index(text_shaper::char_const_iterator it) const noexcept
{
    return narrow<size_t>(std::distance(begin(), it));
}

[[nodiscard]] text_cursor text_shaper::get_begin_cursor() const noexcept
{
    return {};
}

[[nodiscard]] text_cursor text_shaper::get_end_cursor() const noexcept
{
    return {size() - 1, true, size()};
}

[[nodiscard]] text_cursor text_shaper::get_before_cursor(size_t index) const noexcept
{
    return {index, false, size()};
}

[[nodiscard]] text_cursor text_shaper::get_after_cursor(size_t index) const noexcept
{
    return {index, true, size()};
}

[[nodiscard]] text_cursor text_shaper::get_left_cursor(text_shaper::char_const_iterator it) const noexcept
{
    if (it != end()) {
        if (it->direction == unicode_bidi_class::L) {
            return get_before_cursor(it);
        } else {
            return get_after_cursor(it);
        }
    } else {
        return get_end_cursor();
    }
}

[[nodiscard]] text_cursor text_shaper::get_right_cursor(text_shaper::char_const_iterator it) const noexcept
{
    if (it != end()) {
        if (it->direction == unicode_bidi_class::L) {
            return get_after_cursor(it);
        } else {
            return get_before_cursor(it);
        }
    } else {
        return get_end_cursor();
    }
}

[[nodiscard]] bool text_shaper::is_on_left(text_cursor cursor) const noexcept
{
    ttlet it = get_it(cursor);
    if (it != end()) {
        return (it->direction == unicode_bidi_class::L) == cursor.before();
    } else {
        tt_axiom(begin() == end());
        return true;
    }
}

[[nodiscard]] bool text_shaper::is_on_right(text_cursor cursor) const noexcept
{
    ttlet it = get_it(cursor);
    if (it != end()) {
        return (it->direction == unicode_bidi_class::L) == cursor.after();
    } else {
        tt_axiom(begin() == end());
        return true;
    }
}

[[nodiscard]] text_cursor text_shaper::get_nearest_cursor(point2 position) const noexcept
{
    if (_text.empty()) {
        return {};
    }

    ttlet line_it = std::ranges::min_element(_lines, std::ranges::less{}, [position](ttlet &line) {
        return std::abs(line.y - position.y());
    });

    if (line_it != _lines.end()) {
        ttlet[char_it, after] = line_it->get_nearest(position);
        return {narrow<size_t>(std::distance(_text.begin(), char_it)), after, size()};
    } else {
        return {};
    }
}

[[nodiscard]] text_shaper::char_const_iterator text_shaper::move_left_char(text_shaper::char_const_iterator it) const noexcept
{
    ttlet[column_nr, line_nr] = get_column_line(it);
    return get_it(column_nr - 1, line_nr);
}

[[nodiscard]] text_shaper::char_const_iterator text_shaper::move_right_char(text_shaper::char_const_iterator it) const noexcept
{
    ttlet[column_nr, line_nr] = get_column_line(it);
    return get_it(column_nr + 1, line_nr);
}

[[nodiscard]] text_cursor text_shaper::move_left_char(text_cursor cursor, bool overwrite_mode) const noexcept
{
    auto it = get_it(cursor);
    if (overwrite_mode) {
        it = move_left_char(it);
        return get_before_cursor(it);

    } else {
        if (is_on_left(cursor)) {
            // If the cursor is on the left side of a character, then move one character left.
            it = move_left_char(it);
        }

        return get_left_cursor(it);
    }
}

[[nodiscard]] text_cursor text_shaper::move_right_char(text_cursor cursor, bool overwrite_mode) const noexcept
{
    auto it = get_it(cursor);
    if (overwrite_mode) {
        it = move_right_char(it);
        return get_before_cursor(it);

    } else {
        if (is_on_right(cursor)) {
            // If the cursor is on the left side of a character, then move one character left.
            it = move_right_char(it);
        }

        return get_right_cursor(it);
    }
}

[[nodiscard]] text_cursor text_shaper::move_down_char(text_cursor cursor, float &x) const noexcept
{
    if (_text.empty()) {
        return {};
    }

    auto [column_nr, line_nr] = get_column_line(cursor);
    if (++line_nr == _lines.size()) {
        return get_end_cursor();
    }

    if (std::isnan(x)) {
        ttlet char_it = get_it(cursor);
        tt_axiom(char_it != _text.end());
        x = is_on_left(cursor) ? char_it->rectangle.left() : char_it->rectangle.right();
    }

    ttlet[new_char_it, after] = _lines[line_nr].get_nearest(point2{x, 0.0f});
    return get_before_cursor(new_char_it);
}

[[nodiscard]] text_cursor text_shaper::move_up_char(text_cursor cursor, float &x) const noexcept
{
    if (_text.empty()) {
        return {};
    }

    auto [column_nr, line_nr] = get_column_line(cursor);
    if (line_nr-- == 0) {
        return {};
    }

    if (std::isnan(x)) {
        auto char_it = get_it(cursor);
        tt_axiom(char_it < _text.end());
        x = is_on_left(cursor) ? char_it->rectangle.left() : char_it->rectangle.right();
    }

    ttlet[new_char_it, after] = _lines[line_nr].get_nearest(point2{x, 0.0f});
    return get_before_cursor(new_char_it);
}

[[nodiscard]] text_cursor text_shaper::move_left_word(text_cursor cursor, bool overwrite_mode) const noexcept
{
    cursor = move_left_char(cursor, overwrite_mode).before_neighbor(size());
    auto it = get_it(cursor);
    while (it != end()) {
        if (*(it->description) != unicode_general_category::Zs and
            _word_break_opportunities[get_index(it)] != unicode_break_opportunity::no) {
            return get_before_cursor(it);
        }
        it = move_left_char(it);
    }
    return get_end_cursor();
}

[[nodiscard]] text_cursor text_shaper::move_right_word(text_cursor cursor, bool overwrite_mode) const noexcept
{
    cursor = move_right_char(cursor, overwrite_mode).before_neighbor(size());
    auto it = get_it(cursor);
    while (it != end()) {
        if (*(it->description) != unicode_general_category::Zs and
            _word_break_opportunities[get_index(it)] != unicode_break_opportunity::no) {
            return get_before_cursor(it);
        }
        it = move_right_char(it);
    }
    return get_end_cursor();
}

[[nodiscard]] text_cursor text_shaper::move_begin_line(text_cursor cursor) const noexcept
{
    ttlet[column_nr, line_nr] = get_column_line(cursor);
    ttlet &line = _lines[line_nr];
    return get_before_cursor(line.first);
}

[[nodiscard]] text_cursor text_shaper::move_end_line(text_cursor cursor) const noexcept
{
    ttlet[column_nr, line_nr] = get_column_line(cursor);
    ttlet &line = _lines[line_nr];

    auto it = line.last;
    while (it != line.first) {
        --it;
        if (not it->is_trailing_white_space) {
            break;
        }
    }

    return get_after_cursor(it);
}

[[nodiscard]] text_cursor text_shaper::move_begin_sentence(text_cursor cursor) const noexcept
{
    if (cursor.after()) {
        cursor = {cursor.index(), false, size()};
    } else if (cursor.index() != 0) {
        cursor = {cursor.index() - 1, false, size()};
    }
    ttlet[first, last] = select_sentence(cursor);
    return first.before_neighbor(size());
}

[[nodiscard]] text_cursor text_shaper::move_end_sentence(text_cursor cursor) const noexcept
{
    if (cursor.before()) {
        cursor = {cursor.index(), true, size()};
    } else if (cursor.index() != _text.size() - 1) {
        cursor = {cursor.index() + 1, true, size()};
    }
    ttlet[first, last] = select_sentence(cursor);
    return last.before_neighbor(size());
}

[[nodiscard]] text_cursor text_shaper::move_begin_paragraph(text_cursor cursor) const noexcept
{
    if (cursor.after()) {
        cursor = {cursor.index(), false, size()};
    } else if (cursor.index() != 0) {
        cursor = {cursor.index() - 1, false, size()};
    }
    ttlet[first, last] = select_paragraph(cursor);
    return first.before_neighbor(size());
}

[[nodiscard]] text_cursor text_shaper::move_end_paragraph(text_cursor cursor) const noexcept
{
    if (cursor.before()) {
        cursor = {cursor.index(), true, size()};
    } else if (cursor.index() != _text.size() - 1) {
        cursor = {cursor.index() + 1, true, size()};
    }
    ttlet[first, last] = select_paragraph(cursor);
    return last.before_neighbor(size());
}

[[nodiscard]] text_cursor text_shaper::move_begin_document(text_cursor cursor) const noexcept
{
    return {};
}

[[nodiscard]] text_cursor text_shaper::move_end_document(text_cursor cursor) const noexcept
{
    if (_text.empty()) {
        return {};
    }

    return get_end_cursor();
}

[[nodiscard]] std::pair<text_cursor, text_cursor>
text_shaper::get_selection_from_break(text_cursor cursor, unicode_break_vector const &break_opportunities) const noexcept
{
    if (_text.empty()) {
        return {{}, {}};
    }

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

    return {get_before_cursor(first_index), get_after_cursor(last_index)};
}

[[nodiscard]] std::pair<text_cursor, text_cursor> text_shaper::select_char(text_cursor cursor) const noexcept
{
    ttlet index = cursor.index();
    return {get_before_cursor(index), get_after_cursor(index)};
}

[[nodiscard]] std::pair<text_cursor, text_cursor> text_shaper::select_word(text_cursor cursor) const noexcept
{
    return get_selection_from_break(cursor, _word_break_opportunities);
}

[[nodiscard]] std::pair<text_cursor, text_cursor> text_shaper::select_sentence(text_cursor cursor) const noexcept
{
    return get_selection_from_break(cursor, _sentence_break_opportunities);
}

[[nodiscard]] std::pair<text_cursor, text_cursor> text_shaper::select_paragraph(text_cursor cursor) const noexcept
{
    ttlet first_index = [&]() {
        auto i = cursor.index();
        while (i > 0) {
            if (_text[i - 1].description->general_category() == unicode_general_category::Zp) {
                return i;
            }
            --i;
        }
        return i;
    }();
    ttlet last_index = [&]() {
        auto i = cursor.index();
        while (i < _text.size()) {
            if (_text[i].description->general_category() == unicode_general_category::Zp) {
                return i;
            }
            ++i;
        }
        return i;
    }();

    return {get_before_cursor(first_index), get_after_cursor(last_index)};
}

[[nodiscard]] std::pair<text_cursor, text_cursor> text_shaper::select_document(text_cursor cursor) const noexcept
{
    if (_text.empty()) {
        return {{}, {}};
    }

    return {{}, get_end_cursor()};
}

} // namespace tt::inline v1
