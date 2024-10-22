

#pragma once

#include "text_style_set.hpp"
#include "text_style.hpp"
#include "../units/units.hpp"
#include "../unicode/unicode.hpp"
#include "../font/font.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.text : text_guess_size);

hi_export namespace hi::inline v1 {


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
struct shaper_grapheme_metrics {
    font_glyph_ids glyphs = {};
    unit::pixels_f advance = unit::pixels(0.0f);
    unit::pixels_f cap_height = unit::pixels(0.0f);
    unit::pixels_f ascender = unit::pixels(0.0f);
    unit::pixels_f descender = unit::pixels(0.0f);
    unit::pixels_f line_gap = unit::pixels(0.0f);
    float line_spacing = 0.0f;
    float paragraph_spacing = 0.0f;
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
[[nodiscard]] inline std::vector<shaper_grapheme_metrics> shaper_collect_grapheme_metrics(
    gstring_view text,
    std::vector<shaper_run_indices>& run_indices,
    unit::pixels_per_em_f font_size,
    text_style_set const& style_set)
{
    assert(text.size() >= run_indices.size());

    // A scratch pad to share the allocation of this vector between calls to
    // find_glyphs() to improve the performance.
    auto find_glyphs_scratch = std::vector<lean_vector<glyph_id>>{};

    auto r = std::vector<shaper_grapheme_metrics>{};
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

            auto metrics = shaper_grapheme_metrics{};
            metrics.glyphs = glyph_ids;
            metrics.advance = style_font_size * font->get_advance(glyph_ids.front());
            metrics.cap_height = font_metrics.cap_height;
            metrics.ascender = font_metrics.ascender;
            metrics.descender = font_metrics.descender;
            metrics.line_gap = font_metrics.line_gap;
            metrics.line_spacing = style.line_spacing();
            metrics.paragraph_spacing = style.paragraph_spacing();
            metrics.general_category = ucd_get_general_category(g.starter());
            metrics.bracket_type = ucd_get_bidi_paired_bracket_type(g.starter());
            r.push_back(std::move(metrics));
        }
    }

    return r;
}

/** Fold lines of a text.
 * 
 * @param break_opportunities The line break opportunities in the text.
 * @param grapheme_metrics_range The sizing information of each grapheme.
 * @param maximum_line_width The maximum width of a line.
 */
[[nodiscard]] inline std::vector<size_t> shaper_fold_lines(
    unicode_line_break_vector const& break_opportunities,
    std::vector<shaper_grapheme_metrics> const& grapheme_metrics_range,
    unit::pixels_f maximum_line_width)
{
    return unicode_fold_lines(
        break_opportunities,
        grapheme_metrics_range,
        maximum_line_width,
        [](auto const& x) {
            return x.advance;
        },
        [](auto const& x) {
            return is_visible(x.general_category);
        });
}

struct shaper_line_metrics {
    unit::pixels_f cap_height = unit::pixels(0.0f);
    unit::pixels_f ascender = unit::pixels(0.0f);
    unit::pixels_f descender = unit::pixels(0.0f);
    unit::pixels_f line_gap = unit::pixels(0.0f);
    unit::pixels_f advance = unit::pixels(0.0f);
    unit::pixels_f width = unit::pixels(0.0f);
    float spacing = 0.0f;
};

[[nodiscard]] inline std::vector<shaper_line_metrics> shaper_collect_line_metrics(
    std::vector<shaper_grapheme_metrics> const& grapheme_metrics_range,
    std::vector<size_t> const& line_lengths)
{
    auto r = std::vector<shaper_line_metrics>{};
    r.reserve(line_lengths.size());

    auto it = grapheme_metrics_range.begin();
    for (auto const line_length : line_lengths) {
        assert(line_length > 0);

        auto const it_end = it + line_length;
        auto const line_ends_paragraph = (it_end - 1)->general_category == unicode_general_category::Zp;

        auto const it_visible = rfind_if(it, it_end, [](auto const& x) {
            return is_visible(x.general_category);
        });
        auto const it_visible_end = it_visible != it_end ? it_visible + 1 : it_end;
        auto const visible_line_span = std::span{it, it_visible_end};

        auto metrics = shaper_line_metrics{};
        for (auto const& g : visible_line_span) {
            metrics.cap_height = std::max(metrics.cap_height, g.cap_height);
            metrics.ascender = std::max(metrics.ascender, g.ascender);
            metrics.descender = std::max(metrics.descender, g.descender);
            metrics.line_gap = std::max(metrics.line_gap, g.line_gap);
            metrics.spacing = std::max(metrics.spacing, line_ends_paragraph ? g.paragraph_spacing : g.line_spacing);
            metrics.width += g.advance;
        }

        r.push_back(std::move(metrics));
        it = it_end;
    }

    assert(not r.empty());
    for (auto i = size_t{0}; i != r.size() - 1; ++i) {
        r[i].advance = (r[i].descender + std::max(r[i].line_gap, r[i + 1].line_gap) + r[i + 1].ascender) * r[i].spacing;
    }
    r.back().advance = unit::pixels(0.0f);

    return r;
}

struct shaper_text_metrics {
    /** The maximum width of the text.
     */
    unit::pixels_f width = unit::pixels(0.0f);

    /** Heigth from top-line's cap-height, to bottom's base-line.
     */
    unit::pixels_f height = unit::pixels(0.0f);

    /** The ascender minus cap-height of top line.
     */
    unit::pixels_f overhang = unit::pixels(0.0f);

    /** The descender of the bottom line.
     */
    unit::pixels_f underhang = unit::pixels(0.0f);

    /** Function to determine the baseline of the text.
     */
    std::function<unit::pixels_f(unit::pixels_f)> baseline_function = [](unit::pixels_f height) {
        return unit::pixels(0.0f);
    };
};

[[nodiscard]] inline shaper_text_metrics shaper_collect_text_metrics(std::vector<shaper_line_metrics> const &line_metrics, hi::vertical_alignment alignment)
{
    auto r = shaper_text_metrics{};

    if (line_metrics.empty()) {
        return r;
    }

    auto const ascender = line_metrics.front().ascender;
    auto const cap_height = line_metrics.front().ascender;
    auto const descender = line_metrics.back().descender;

    r.height = cap_height;
    for (auto const &m : line_metrics) {
        r.width = std::max(r.width, m.width);
        r.height += m.advance;
    }

    // The overhang and underhang are used to determine the margins around the
    // text.
    if (ascender > cap_height) {
        r.overhang = ascender - cap_height;
    }
    r.underhang = descender;

    // The baseline of the middle line, or the average of the two middle lines.
    // The distance from the bottom of the text to the baseline.
    auto const middle_baseline_from_bottom = r.height - [&]{
        auto const i = line_metrics.size() % 2 == 0 ? (line_metrics.size() / 2 - 1) : line_metrics.size() / 2;

        auto const y = std::accumulate(line_metrics.begin(), line_metrics.begin() + i + 1, unit::pixels(0.0f), [](auto sum, auto const& m) {
            return sum + m.advance;
        });

        if (line_metrics.size() % 2 == 0) {
            auto const y2 = y + line_metrics[i].advance;
            return (y + y2) / 2.0f;
        } else {
            return y;
        }
    }();

    // Calculate the offset to the middle baseline from the middle of the text.
    auto const middle_baseline_from_middle = middle_baseline_from_bottom - r.height * 0.5f;

    r.baseline_function = [&]() -> std::function<unit::pixels_f(unit::pixels_f)> {
        switch (alignment) {
        case vertical_alignment::top:
            return [=](unit::pixels_f height) {
                return height - cap_height;
            };

        case vertical_alignment::middle:
            return [=](unit::pixels_f height) {
                return height * 0.5f + middle_baseline_from_middle;
            };

        case vertical_alignment::bottom:
            return [](unit::pixels_f height) {
                return unit::pixels(0.0f);
            };
        }
        std::unreachable();
    }();

    return r;
}

}
