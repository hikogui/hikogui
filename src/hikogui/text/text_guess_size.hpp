

#pragma once

#include "text_style_set.hpp"
#include "text_style.hpp"
#include "../units/units.hpp"
#include "../unicode/unicode.hpp"
#include "../font/font.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.text : text_guess_size);

hi_export namespace hi::inline v1 {
// struct fold_text_result {
//     /** The width of the text.
//      *
//      * This is the width of the text after it has been folded to fit within the
//      * maximum width.
//      */
//     unit::pixels_f width = unit::pixels(0.0f);
//
//     /** The height of the text.
//      *
//      * The height is from the baseline from the top line to the baseline of the
//      * bottom line. This is the height of the text after it has been folded to
//      * fit within the maximum width.
//      */
//     unit::pixels height = unit::pixels(0.0f);
//
//     /** The cap-height of the top line.
//      */
//     unit::pixels top_cap_height = unit::pixels(0.0f);
//
//     /** The size of the ascender of the top line.
//      */
//     unit::pixels top_ascender = unit::pixels(0.0f);
//
//     /** The size of the descender of the bottom line.
//      */
//     unit::pixels bottom_descender = unit::pixels(0.0f);
//
//     /** The number of graphemes for each line.
//      */
//     std::vector<size_t> line_sizes = {};
// };

/** Get the width of each grapheme.
 *
 * @param text The text to get the width of each grapheme.
 * @param font_size The size of the font.
 * @param style_set The style of the text.
 * @return The width of each grapheme.
 */
[[nodiscard]] std::vector<float>
shaper_grapheme_widths(gstring const& text, unit::pixels_per_em_f font_size, text_style_set const& style_set)
{
    auto r = std::vector<float>{};
    if (text.empty()) {
        return r;
    }

    r.reserve(text.size());

    auto attributes = text.front().attributes();
    auto style = style_set[attributes];
    auto style_font_size = font_size * style.scale();
    auto font_id = style.font_chain().front();
    auto font = std::addressof(get_font(font_id));

    for (auto const& c : text) {
        if (auto const a = c.attributes(); a != attributes) {
            attributes = a;
            style = style_set[a];
            font_id = style.font_chain().front();
            font = std::addressof(get_font(font_id));
        }

        auto const starter_cp = c.starter();

        auto const width = [&]() -> unit::pixels_f {
            for (auto f_id : style.font_chain()) {
                auto const& f = f_id == font_id ? *font : get_font(f_id);
                if (auto const glyph_id = f.find_glyph(starter_cp); glyph_id) {
                    return unit::em_squares(f.get_advance(glyph_id)) * style_font_size;
                }
            }

            // Unknown character, tofu.
            return unit::em_squares(font->get_advance(glyph_id{0})) * style_font_size;
        }();

        auto const general_category = ucd_get_general_category(starter_cp);
        r.push_back(is_visible(general_category) ? width.in(unit::pixels) : -width.in(unit::pixels));
    }

    return r;
}

[[nodiscard]] extent2 shaper_text_size(
    gstring const& text,
    std::vector<float> const& widths,
    std::vector<size_t> const& line_lengths,
    unit::pixels_per_em_f font_size,
    text_style_set const& style_set) noexcept
{
    assert(widths.size() == text.size());

    auto r = extent2{};
    if (widths.empty()) {
        return r;
    }

    auto text_it = text.begin();
    auto widths_it = widths.begin();
    auto text_width = 0.0f;
    auto text_height = 0.0f;
    for (auto const line_length : line_lengths) {
        assert(std::distance(widths_it, widths.end()) >= line_length);
        assert(std::distance(text_it, text.end()) >= line_length);

        // Calculate the length of the line, excluding any trailing whitespace.
        auto const line_length_wtw = [&]() -> size_t{
            for (auto i = line_length; i != 0; --i) {
                if (*(widths_it + i - 1) >= 0.0f) {
                    return i;
                }
            }
            return 0;
        }();

        auto const line_width = std::reduce(widths_it, widths_it + line_length_wtw, 0.0f, [](auto a, auto b) {
            return a + std::abs(b);
        });
        text_width = std::max(text_width, line_width);

        // The first grapheme in the line determines the line metrics for the
        // whole line. If there is no grapheme in the line, the line metrics
        // are determined from the first grapheme in the text.
        auto const first_g = line_length > 0 ? *text_it : text.front();
        auto const &style = style_set[first_g.attributes()];
        auto const &font = get_font(style.font_chain().front());
        auto const &metrics = font.metrics * (font_size * style.scale());
        

        // The last grapheme in the line determines if it is the end of a line
        // or a paragraph. And thus determines the distance to the next line.
        // The last line may not have a trailing end-of-line/paragraph character,
        // in this case that line ends a paragraph.
        auto const last_cp = line_length > 0 ? (text_it + line_length - 1)->starter() : U'\u2029';
        auto const is_paragraph = ucd_get_general_category(last_cp) == unicode_general_category::Zp;
        auto const line_advance = advance * (is_paragraph ? style.paragraph_spacing() : style.line_spacing());

        widths_it += line_length;
        text_it += line_length;
    }

    auto const line_count = line_sizes.size();
    auto const line_height_total = line_height * line_count;
    auto const line_gap_total = line_gap * (line_count - 1);
    auto const ascender_total = ascender * line_count;
    auto const descender_total = descender * line_count;

    r.width = std::accumulate(widths.begin(), widths.end(), 0.0f);
    r.height = line_height_total + line_gap_total + ascender_total + descender_total;

    return r;
}
//
// struct shaper_line_metrics {
//    unit::pixels cap_height = unit::pixels(0.0f);
//    unit::pixels ascender = unit::pixels(0.0f);
//    unit::pixels descender = unit::pixels(0.0f);
//    unit::pixels line_gap = unit::pixels(0.0f);
//    unit::pixels vertical_advance = unit::pixels(0.0f);
//};

///** Guess the size of text.
// *
// * @param text The text to guess the size of.
// * @param line_break_opportunities The line break opportunities in the text.
// * @param font_size The size of the font.
// * @param style_set The style of the text.
// * @param max_width The maximum width of the text.
// * @return Information about the size of the text.
// */
//[[nodiscard]] fold_text_result fold_text(
//    std::gstring_view text,
//    unicode_break_vector const& line_break_opportunities,
//    unit::pixels font_size,
//    text_style_set const& style_set,
//    float max_width) noexcept
//{
//    auto r = fold_text_result{};
//
//    if (text.empty()) {
//        return r;
//    }
//
//    _line_break_widths.reserve(text.size());
//    for (auto const& c : _text) {
//        _line_break_widths.push_back(is_visible(c.general_category) ? c.width : -c.width);
//    }
//
//    auto const line_sizes = unicode_line_break(line_break_opportunities, _line_break_widths, rectangle.width());
//
//    grapheme_attributes attributes = text.front().attributes();
//    auto style = style_set.get_style(attributes);
//
//    auto it = text.begin();
//    auto word_start = it;
//    for (auto it = text.begin(); it != text.end(); ++it) {}
//
//    return r;
//}
}
