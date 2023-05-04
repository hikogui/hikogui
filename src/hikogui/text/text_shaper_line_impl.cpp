// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_shaper_line.hpp"
#include "../unicode/module.hpp"

namespace hi::inline v1 {

text_shaper_line::text_shaper_line(
    size_t line_nr,
    const_iterator begin,
    iterator first,
    iterator last,
    float width,
    hi::font_metrics const& metrics) noexcept :
    first(first), last(last), columns(), metrics(metrics), line_nr(line_nr), y(0.0f), width(width), last_category()
{
    auto last_visible_it = first;
    for (auto it = first; it != last; ++it) {
        // Reset the trailing white space marker.
        it->is_trailing_white_space = false;

        // Only calculate line metrics based on visible characters.
        // For example a paragraph separator is seldom available in a font.
        auto general_category = ucd_get_general_category(it->character[0]);
        if (is_visible(general_category)) {
            this->metrics = max(metrics, it->font->metrics);
            last_visible_it = it;
        }
    }

    if (first != last) {
        // Mark trailing whitespace as such
        for (auto it = last_visible_it + 1; it != last; ++it) {
            it->is_trailing_white_space = true;
        }

        last_category = ucd_get_general_category((last - 1)->character[0]);
    } else {
        last_category = unicode_general_category::Cn;
    }
}

static void advance_glyphs_run(point2 &p, text_shaper_line::column_vector::iterator first, text_shaper_line::column_vector::iterator last) noexcept
{
    hi_axiom(first != last);

    hilet char_it = *first;
    hilet& font = *char_it->font;
    hilet attributes = char_it->character.attributes();
    hilet script = attributes.script();
    hilet style = char_it->style;

    auto run = gstring{};
    run.reserve(std::distance(first, last));
    for (auto it = first; it != last; ++it) {
        run += (*it)->character.grapheme();
    }

    auto result = font.shape_run(attributes.language(), script, run);
    result.scale(style.size);
    hi_axiom(result.advances.size() == run.size());
    hi_axiom(result.glyph_count.size() == run.size());

    auto grapheme_index = 0_uz;
    auto glyph_index = 0_uz;
    for (auto it = first; it != last; ++it, ++grapheme_index) {
        auto &c = **it;

        c.position = p;
        c.advance = result.advances[grapheme_index];
        c.glyphs.clear();
        c.glyph_rectangles.clear();

        hilet glyph_count = result.glyph_count[grapheme_index];
        for (auto j = 0_uz; j != glyph_count; ++j, ++glyph_index) {
            hi_axiom_bounds(glyph_index, result.glyphs);
            c.glyphs.push_back(result.glyphs[glyph_index]);

            hi_axiom_bounds(glyph_index, result.glyph_rectangles);
            c.glyph_rectangles.push_back(result.glyph_rectangles[glyph_index]);
        }

        p += vector2{c.advance, 0.0f};
    }
}

/**
 */
static void advance_glyphs(text_shaper_line::column_vector& columns, float y) noexcept
{
    if (columns.empty()) {
        return;
    }

    auto p = point2{0.0f, y};

    auto run_start = columns.begin();
    for (auto it = run_start + 1; it != columns.end(); ++it) {
        hilet start_char_it = *run_start;
        hilet char_it = *it;

        hilet same_font = start_char_it->font == char_it->font;
        hilet same_attributes = start_char_it->character.attributes() == char_it->character.attributes();

        if (not(same_font and same_attributes)) {
            advance_glyphs_run(p, run_start, it);
            run_start = it;
        }
    }
    advance_glyphs_run(p, run_start, columns.end());
}

[[nodiscard]] static std::pair<float, size_t>
calculate_precise_width(text_shaper_line::column_vector& columns, unicode_bidi_class paragraph_direction)
{
    if (columns.empty()) {
        return {0.0f, 0_uz};
    }

    auto it = columns.begin();
    for (; it != columns.end(); ++it) {
        if (not(*it)->is_trailing_white_space) {
            break;
        }
    }
    hilet left_x = (*it)->position.x();

    auto right_x = left_x;
    auto num_white_space = 0_uz;
    for (; it != columns.end(); ++it) {
        if ((*it)->is_trailing_white_space) {
            // Stop at the first trailing white space.
            break;
        }

        right_x = (*it)->position.x() + (*it)->advance;
        hilet general_category = ucd_get_general_category((*it)->character[0]);
        if (not is_visible(general_category)) {
            ++num_white_space;
        }
    }

    hilet width = right_x - left_x;

    // Adjust the offset to left align on the first visible character.
    for (auto& char_it : columns) {
        char_it->position.x() -= left_x;
    }

    return {width, num_white_space};
}

static void move_glyphs(text_shaper_line::column_vector& columns, float offset) noexcept
{
    for (hilet& char_it : columns) {
        char_it->position.x() += offset;
    }
}

[[nodiscard]] static bool align_glyphs_justified(
    text_shaper_line::column_vector& columns,
    float max_line_width,
    float visible_width,
    size_t num_internal_white_space) noexcept
{
    if (num_internal_white_space == 0) {
        return false;
    }

    hilet extra_space = max_line_width - visible_width;
    if (extra_space > max_line_width * 0.25f) {
        return false;
    }

    hilet extra_space_per_whitespace = extra_space / num_internal_white_space;
    auto offset = 0.0f;
    for (hilet& char_it : columns) {
        char_it->position.x() += offset;

        // Add extra space for each white space in the visible part of the line. Leave the
        // sizes of trailing white space normal.
        hilet general_category = ucd_get_general_category(char_it->character[0]);
        if (not char_it->is_trailing_white_space and not is_visible(general_category)) {
            offset += extra_space_per_whitespace;
        }
    }

    return true;
}

static void align_glyphs(
    text_shaper_line::column_vector& columns,
    horizontal_alignment alignment,
    unicode_bidi_class paragraph_direction,
    float max_line_width,
    float visible_width,
    size_t num_internal_white_space) noexcept
{
    if (alignment == horizontal_alignment::justified) {
        if (align_glyphs_justified(columns, max_line_width, visible_width, num_internal_white_space)) {
            return;
        }
    }

    if (alignment == horizontal_alignment::flush or alignment == horizontal_alignment::justified) {
        alignment = paragraph_direction == unicode_bidi_class::R ? horizontal_alignment::right : horizontal_alignment::left;
    }

    // clang-format off
    hilet offset =
        alignment == horizontal_alignment::left ? 0.0f :
        alignment == horizontal_alignment::right ? max_line_width - visible_width :
        (max_line_width - visible_width) * 0.5f;
    // clang-format on

    return move_glyphs(columns, offset);
}

static void round_glyph_positions(text_shaper_line::column_vector& columns, float sub_pixel_width) noexcept
{
    hilet rcp_sub_pixel_width = 1.0f / sub_pixel_width;
    for (auto it : columns) {
        it->position.x() = std::round(it->position.x() * rcp_sub_pixel_width) * sub_pixel_width;
    }
}

static void
create_bounding_rectangles(text_shaper_line::column_vector& columns, float y, float ascender, float descender) noexcept
{
    for (auto it = columns.begin(); it != columns.end(); ++it) {
        hilet next_it = it + 1;
        hilet char_it = *it;
        if (next_it == columns.end()) {
            char_it->rectangle = {
                point2{char_it->position.x(), y - descender},
                point2{char_it->position.x() + char_it->advance, y + ascender}};
        } else {
            hilet next_char_it = *next_it;
            char_it->rectangle = {point2{char_it->position.x(), y - descender}, point2{next_char_it->position.x(), y + ascender}};
        }
    }
}

void text_shaper_line::layout(horizontal_alignment alignment, float min_x, float max_x, float sub_pixel_width) noexcept
{
    // Reset the position and advance the glyphs.
    advance_glyphs(columns, y);

    // Calculate the precise width of the line.
    hilet[visible_width, num_internal_white_space] = calculate_precise_width(columns, paragraph_direction);

    // Align the glyphs for a given width. But keep the left side at x=0.0.
    align_glyphs(columns, alignment, paragraph_direction, max_x - min_x, visible_width, num_internal_white_space);

    // Move the glyphs to where the left side is.
    move_glyphs(columns, min_x);

    // Round the glyphs to sub-pixels to improve sharpness of rendered glyphs.
    round_glyph_positions(columns, sub_pixel_width);

    // Create the bounding rectangles around each glyph, for use to draw selection boxes/cursors and handle mouse control.
    create_bounding_rectangles(columns, y, metrics.ascender, metrics.descender);

    // Create a bounding rectangle around the visible part of the line.
    if (columns.empty()) {
        rectangle = {point2{0.0f, y - metrics.descender}, point2{1.0f, y + metrics.ascender}};
    } else {
        rectangle = columns.front()->rectangle | columns.back()->rectangle;
    }
}

[[nodiscard]] std::pair<text_shaper_line::const_iterator, bool> text_shaper_line::get_nearest(point2 position) const noexcept
{
    if (columns.empty()) {
        // This is the last line, so return an the iterator to the end-of-document.
        return {last, false};
    }

    auto column_it = std::lower_bound(columns.begin(), columns.end(), position.x(), [](hilet& char_it, hilet& x) {
        return char_it->rectangle.right() < x;
    });
    if (column_it == columns.end()) {
        column_it = columns.end() - 1;
    }

    auto char_it = *column_it;
    hilet general_category = ucd_get_general_category(char_it->character[0]);
    if (is_Zp_or_Zl(general_category)) {
        // Do not put the cursor on a paragraph separator or line separator.
        if (paragraph_direction == unicode_bidi_class::L) {
            if (column_it != columns.begin()) {
                char_it = *--column_it;
            } else {
                // If there is only a paragraph separator, place the cursor before it.
                return {char_it, false};
            }
        } else {
            if (column_it + 1 != columns.end()) {
                char_it = *++column_it;
            } else {
                // If there is only a paragraph separator, place the cursor before it.
                return {char_it, false};
            }
        }
    }

    hilet after = (char_it->direction == unicode_bidi_class::L) == position.x() > char_it->rectangle.center();
    return {char_it, after};
}

} // namespace hi::inline v1
