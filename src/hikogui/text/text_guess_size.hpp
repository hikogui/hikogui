

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

struct shaper_run_indices {
    size_t first = 0;
    size_t last = 0;
};

/** Get runs of text.
 *
 * A run is a sequence of graphemes that are not separated by a word break and
 * have the same grapheme attributes. Which means that the graphemes of a run
 * can be shaped together. A sequence of whitespace is considered a run.
 *
 * @param text The text to get the runs of.
 * @param word_breaks The word breaks in the text.
 * @return A vector of pairs of the start and one-past-the-end index of each run.
 */
[[nodiscard]] inline std::vector<shaper_run_indices>
shaper_make_run_indices(gstring_view text, unicode_word_break_vector word_breaks)
{
    auto r = std::vector<shaper_run_indices>{};

    // If the text is empty, then word_breaks is also empty.
    if (text.empty()) {
        return r;
    }

    assert(text.size() + 1 == word_breaks.size());
    // The last character is always a word break. Therfore we do not need to
    // bound check the next grapheme access inside the loop.
    assert(word_breaks.back() == unicode_break_opportunity::yes);

    auto run_start = size_t{0};
    for (auto i = size_t{0}; i != text.size(); ++i) {
        // Is this is the last character of the run.
        if (word_breaks[i + 1] == unicode_break_opportunity::yes) {
            r.emplace_back(run_start, i + 1);
            run_start = i + 1;

        } else if (text[i + 1].attributes() != text[i].attributes()) { // No bound check, see assert above.
            r.emplace_back(run_start, i + 1);
            run_start = i + 1;
        }
    }

    return r;
}

/** Information gathered from a grapheme in preparation for shaping.
 */
struct shaper_grapheme_info {
    font_glyph_ids glyphs = {};
    unit::pixels_f advance = unit::pixels(0.0f);
    unit::pixels_f cap_height = unit::pixels(0.0f);
    unit::pixels_f ascender = unit::pixels(0.0f);
    unit::pixels_f descender = unit::pixels(0.0f);
    unit::pixels_f line_gap = unit::pixels(0.0f);
    unicode_general_category general_category = unicode_general_category::Cn;
    unicode_bidi_paired_bracket_type bracket_type = unicode_bidi_paired_bracket_type::n;
};

/** Get information of each grapheme in preparation for shaping.
 *
 * @param text The text to get the width of each grapheme.
 * @param run_indices The indices of the runs in the text.
 * @param font_size The size of the font.
 * @param style_set The style of the text.
 * @return Sizing information about each grapheme.
 */
[[nodiscard]] inline std::vector<shaper_grapheme_info> shaper_collect_grapheme_info(
    gstring_view text,
    std::vector<shaper_run_indices>& run_indices,
    unit::pixels_per_em_f font_size,
    text_style_set const& style_set)
{
    assert(text.size() >= run_indices.size());

    // A scratch pad to share the allocation of this vector between calls to
    // find_glyphs() to improve the performance.
    auto find_glyphs_scratch = std::vector<lean_vector<glyph_id>>{};

    auto r = std::vector<shaper_grapheme_info>{};
    r.reserve(text.size());

    auto attributes = grapheme_attributes{};
    text_style style = text_style{};
    auto style_font_size = font_size;
    for (auto [first, last] : run_indices) {
        assert(first < last);

        auto const run = text.substr(first, last - first);
        if (style.empty() or run.front().attributes() != attributes) {
            attributes = run.front().attributes();
            style = style_set[attributes];
            style_font_size = font_size * style.scale();
        }

        auto const run_glyphs = find_glyphs(run, style.font_chain(), find_glyphs_scratch);
        auto font_id = hi::font_id{};
        hi::font* font = nullptr;
        auto font_metrics = hi::font_metrics_px{};
        for (auto i = size_t{0}; i != run.size(); ++i) {
            auto const g = run[i];
            auto const glyph_ids = run_glyphs[i];

            if (font == nullptr or font_id != glyph_ids.font) {
                font_id = glyph_ids.font;
                font = std::addressof(get_font(font_id));
                font_metrics = style_font_size * font->metrics;
            }

            auto grapheme_info = shaper_grapheme_info{};
            grapheme_info.glyphs = glyph_ids;
            grapheme_info.advance = style_font_size * font->get_advance(glyph_ids.front());
            grapheme_info.cap_height = font_metrics.cap_height;
            grapheme_info.ascender = font_metrics.ascender;
            grapheme_info.descender = font_metrics.descender;
            grapheme_info.line_gap = font_metrics.line_gap;
            grapheme_info.general_category = ucd_get_general_category(g.starter());
            grapheme_info.bracket_type = ucd_get_bidi_paired_bracket_type(g.starter());
            r.push_back(std::move(grapheme_info));
        }
    }

    return r;
}

/** Fold lines of a text.
 * 
 * @param break_opportunities The line break opportunities in the text.
 * @param grapheme_info_range The sizing information of each grapheme.
 * @param maximum_line_width The maximum width of a line.
 */
[[nodiscard]] std::vector<size_t> shaper_fold_lines(
    unicode_line_break_vector const& break_opportunities,
    std::vector<shaper_grapheme_info> const& grapheme_info_range,
    unit::pixels_f maximum_line_width)
{
    return unicode_fold_lines(
        break_opportunities,
        grapheme_info_range,
        maximum_line_width,
        [](auto const& x) {
            return x.advance;
        },
        [](auto const& x) {
            return is_visible(x.general_category);
        });
}

//[[nodiscard]] extent2 shaper_text_size(
//    gstring const& text,
//    std::vector<float> const& widths,
//    std::vector<size_t> const& line_lengths,
//    unit::pixels_per_em_f font_size,
//    text_style_set const& style_set) noexcept
//{
//    assert(widths.size() == text.size());
//
//    auto r = extent2{};
//    if (widths.empty()) {
//        return r;
//    }
//
//    auto text_it = text.begin();
//    auto widths_it = widths.begin();
//    auto text_width = 0.0f;
//    auto text_height = 0.0f;
//    for (auto const line_length : line_lengths) {
//        assert(std::distance(widths_it, widths.end()) >= line_length);
//        assert(std::distance(text_it, text.end()) >= line_length);
//
//        // Calculate the length of the line, excluding any trailing whitespace.
//        auto const line_length_wtw = [&]() -> size_t {
//            for (auto i = line_length; i != 0; --i) {
//                if (*(widths_it + i - 1) >= 0.0f) {
//                    return i;
//                }
//            }
//            return 0;
//        }();
//
//        auto const line_width = std::reduce(widths_it, widths_it + line_length_wtw, 0.0f, [](auto a, auto b) {
//            return a + std::abs(b);
//        });
//        text_width = std::max(text_width, line_width);
//
//        // The first grapheme in the line determines the line metrics for the
//        // whole line. If there is no grapheme in the line, the line metrics
//        // are determined from the first grapheme in the text.
//        auto const first_g = line_length > 0 ? *text_it : text.front();
//        auto const& style = style_set[first_g.attributes()];
//        auto const& font = get_font(style.font_chain().front());
//        auto const& metrics = font.metrics * (font_size * style.scale());
//
//        // The last grapheme in the line determines if it is the end of a line
//        // or a paragraph. And thus determines the distance to the next line.
//        // The last line may not have a trailing end-of-line/paragraph character,
//        // in this case that line ends a paragraph.
//        auto const last_cp = line_length > 0 ? (text_it + line_length - 1)->starter() : U'\u2029';
//        auto const is_paragraph = ucd_get_general_category(last_cp) == unicode_general_category::Zp;
//        auto const line_advance = advance * (is_paragraph ? style.paragraph_spacing() : style.line_spacing());
//
//        widths_it += line_length;
//        text_it += line_length;
//    }
//
//    auto const line_count = line_sizes.size();
//    auto const line_height_total = line_height * line_count;
//    auto const line_gap_total = line_gap * (line_count - 1);
//    auto const ascender_total = ascender * line_count;
//    auto const descender_total = descender * line_count;
//
//    r.width = std::accumulate(widths.begin(), widths.end(), 0.0f);
//    r.height = line_height_total + line_gap_total + ascender_total + descender_total;
//
//    return r;
//}
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
