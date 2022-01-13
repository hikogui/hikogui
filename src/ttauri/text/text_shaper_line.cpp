// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_shaper_line.hpp"
#include "unicode_line_break.hpp"

namespace tt::inline v1 {

text_shaper_line::text_shaper_line(const_iterator begin, const_iterator first, const_iterator last) noexcept :
    first(first), last(last), columns(), metrics(), y(0.0f), width(0.0f), end_of_paragraph(false)
{
    tt_axiom(first != last);

    width = unicode_LB_width(
        first,
        last,
        [](ttlet &c) {
            return c.description->general_category();
        },
        [](ttlet &c) {
            return c.width;
        });

    for (auto it = first; it != last; ++it) {
        metrics = max(metrics, it->font_metrics());
    }

    ttlet last_category = (last - 1)->description->general_category();
    if (last_category == unicode_general_category::Zp) {
        end_of_paragraph = true;
    }
}

/**
 */
static std::pair<float, size_t> advance_glyphs(text_shaper_line::column_vector &columns, float y) noexcept
{
    auto visible_width = 0.0f;
    auto num_internal_white_space = 0_uz;

    auto p = point2{0.0f, y};
    auto num_white_space = 0_uz;
    for (auto it = columns.begin(); it != columns.end(); ++it) {
        ttlet char_it = *it;
        ttlet next_it = it + 1;
        ttlet kerning = next_it == columns.end() ? vector2{} : char_it->get_kerning(**next_it);

        char_it->position = p;
        p += char_it->metrics.advance + kerning;

        if (is_visible(char_it->description->general_category())) {
            visible_width = p.x();
            num_internal_white_space = num_white_space;
        } else {
            ++num_white_space;
        }
    }
    return {visible_width, num_internal_white_space};
}

static void move_glyphs(text_shaper_line::column_vector &columns, float offset) noexcept
{
    for (ttlet &char_it : columns) {
        char_it->position.x() += offset;
    }
}

[[nodiscard]] static bool align_glyphs_justified(
    text_shaper_line::column_vector &columns,
    float max_line_width,
    float visible_width,
    size_t num_internal_white_space) noexcept
{
    if (num_internal_white_space == 0) {
        return false;
    }

    ttlet extra_space = max_line_width - visible_width;
    if (extra_space > max_line_width * 0.10f) {
        return false;
    }

    ttlet extra_space_per_whitespace = extra_space / num_internal_white_space;
    auto offset = 0.0f;
    for (ttlet &char_it : columns) {
        char_it->position.x() += offset;

        // Add extra space for each white space in the visible part of the line. Leave the
        // sizes of trailing white space normal.
        if (char_it->position.x() < visible_width and not is_visible(char_it->description->general_category())) {
            offset += extra_space_per_whitespace;
        }
    }

    return true;
}

static void align_glyphs(
    text_shaper_line::column_vector &columns,
    horizontal_alignment alignment,
    float max_line_width,
    float visible_width,
    size_t num_internal_white_space) noexcept
{
    if (alignment == horizontal_alignment::justified) {
        if (align_glyphs_justified(columns, max_line_width, visible_width, num_internal_white_space)) {
            return;
        }
    }

    // clang-format off
    ttlet offset =
        alignment == horizontal_alignment::left ? 0.0f :
        alignment == horizontal_alignment::right ? max_line_width - visible_width :
        (max_line_width - visible_width) * 0.5f;
    // clang-format on

    return move_glyphs(columns, offset);
}

static void round_glyph_positions(text_shaper_line::column_vector &columns, float sub_pixel_width) noexcept
{
    ttlet rcp_sub_pixel_width = 1.0f / sub_pixel_width;
    for (auto it : columns) {
        it->position.x() = std::round(it->position.x() * rcp_sub_pixel_width) * sub_pixel_width;
    }
}

static void create_bounding_rectangles(text_shaper_line::column_vector &columns, float ascender, float descender) noexcept
{
    for (auto it = columns.begin(); it != columns.end(); ++it) {
        ttlet next_it = it + 1;
        ttlet char_it = *it;
        if (next_it == columns.end()) {
            char_it->rectangle = {
                point2{char_it->position.x(), char_it->position.y() - descender},
                point2{char_it->position.x() + char_it->metrics.advance.x(), char_it->position.y() + ascender}};
        } else {
            ttlet next_char_it = *next_it;
            char_it->rectangle = {
                point2{char_it->position.x(), char_it->position.y() - descender},
                point2{next_char_it->position.x(), char_it->position.y() + ascender}};
        }
    }
}

void text_shaper_line::layout(horizontal_alignment alignment, float min_x, float max_x, float sub_pixel_width) noexcept
{
    if (alignment == horizontal_alignment::flush or alignment == horizontal_alignment::justified) {
        alignment = paragraph_direction == unicode_bidi_class::R ? horizontal_alignment::right : horizontal_alignment::left;
    }

    // Reset the position and advance the glyphs.
    ttlet[visible_width, num_internal_white_space] = advance_glyphs(columns, y);

    // Align the glyphs for a given width. But keep the left side at x=0.0.
    align_glyphs(columns, alignment, max_x - min_x, visible_width, num_internal_white_space);

    // Move the glyphs to where the left side is.
    move_glyphs(columns, min_x);

    // Round the glyphs to sub-pixels to improve sharpness of rendered glyphs.
    round_glyph_positions(columns, sub_pixel_width);

    // Create the bounding rectangles around each glyph, for use to draw selection boxes/cursors and handle mouse control.
    create_bounding_rectangles(columns, metrics.ascender, metrics.descender);

    // Create a bounding rectangle around the visible part of the line.
    rectangle = not columns.empty() ? columns.front()->rectangle | columns.back()->rectangle : aarectangle{};
}

} // namespace tt::inline v1
