// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_shaper.hpp"
#include "font_book.hpp"
#include "../log.hpp"

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

void text_shaper::char_type::replace_glyph(tt::font_book &font_book, char32_t code_point) noexcept
{
    ttlet &font = font_book.find_font(style.family_id, style.variant);
    set_glyph(font_book.find_glyph(font, tt::grapheme{code_point}));

    bounding_rectangle = metrics.bounding_rectangle;
    glyph_is_initial = false;
}

[[nodiscard]] text_shaper::text_shaper(
    tt::font_book &font_book,
    gstring const &text,
    text_style const &style,
    tt::vertical_alignment vertical_alignment,
    float line_spacing,
    float paragraph_spacing) noexcept :
    _font_book(&font_book),
    _vertical_alignment(vertical_alignment),
    _line_spacing(line_spacing),
    _paragraph_spacing(paragraph_spacing)
{
    ttlet &font = font_book.find_font(style.family_id, style.variant);
    _text.reserve(text.size());
    for (ttlet &c : text) {
        auto &tmp = _text.emplace_back(c, style);
        tmp.initialize_glyph(font_book, font);
    }
}

[[nodiscard]] text_shaper::text_shaper(
    font_book &font_book,
    std::string_view text,
    text_style const &style,
    tt::vertical_alignment vertical_alignment,
    float line_spacing,
    float paragraph_spacing) noexcept :
    text_shaper(font_book, to_gstring(text), style, vertical_alignment, line_spacing, paragraph_spacing)
{
}

[[nodiscard]] generator<ssize_t> text_shaper::fold(float width) const noexcept
{
    auto word_begin = 0_uz;
    auto word_width = 0.0f;
    auto line_width = 0.0f;
    auto index = 0_uz;

    auto prev_category = unicode_general_category::Zp;
    while (index != _text.size()) {
        ttlet &c = _text[index];
        ttlet category = c.description->general_category();

        if (category == unicode_general_category::Zp or category == unicode_general_category::Zl) {
            // Found a line or paragraph separator; add all the characters including the separator.
            for (auto i = word_begin; i != (index + 1); ++i) {
                co_yield static_cast<ssize_t>(i);
            }
            line_width = 0.0f;
            word_begin = ++index;
            word_width = 0.0f;

        } else if (category == unicode_general_category::Zs) {
            // Found a space; add all the characters including the space.
            for (auto i = word_begin; i != (index + 1); ++i) {
                co_yield static_cast<ssize_t>(i);
            }

            // Extent the line with the word upto and including the space.
            // The new word starts at the space.
            // Add the length of the space to the new word.
            line_width += word_width + c.width;
            word_begin = ++index;
            word_width = 0.0f;

        } else if (line_width == 0.0f and word_width + c.width > width) {
            // The word by itself on the line is too large. Just continue and wait for a white-space.
            word_width += c.width;
            ++index;

        } else if (line_width + word_width + c.width > width) {
            // Adding another character to the line makes it too long.
            // Break the line at the begin of the word.
            co_yield -1;

            // Start at a new line.
            line_width = 0.0f;

            // retry the character on the next iteration. Don't increment index.

        } else {
            // Add the new character to the word.
            word_width += c.width;
            ++index;
        }
    }

    // Add the last word
    for (auto i = word_begin; i != _text.size(); ++i) {
        co_yield static_cast<ssize_t>(i);
    }

    // If the text does not have a paragraph separator at the end; add one here.
    if (_text.back().description->general_category() != unicode_general_category::Zp) {
        co_yield -2;
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
