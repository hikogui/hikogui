// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;

#include <span>
#include <vector>
#include <map>
#include <string>

export module hikogui_font_font;
import hikogui_container;
import hikogui_font_font_char_map;
import hikogui_font_font_metrics;
import hikogui_font_font_variant;
import hikogui_font_font_weight;
import hikogui_font_glyph_atlas_info;
import hikogui_font_glyph_metrics;
import hikogui_graphic_path;
import hikogui_i18n;
import hikogui_unicode;
import hikogui_utility;

export namespace hi::inline v1 {

/*! A font.
 * This class has information on how to shape text and
 * get glyphs consisting of bezier contours.
 */
export class font {
public:
    /** The family name as parsed from the font file.
     *
     * Examples: "Helvetica", "Times New Roman"
     */
    std::string family_name;

    /** The sub-family name as parsed from the font file.
     *
     * Examples: "Regular", "ItalicBold"
     */
    std::string sub_family_name;

    bool monospace = false;
    bool serif = false;
    font_style style = font_style::normal;
    bool condensed = false;
    font_weight weight = font_weight::regular;
    float optical_size = 12.0;

    /** A optimized character map.
     *
     * This character map is always available even if the font is not loaded.
     */
    font_char_map char_map;

    /** A string representing the features of a font.
     * This will be a comma separated list of features, mostly tables like 'kern' and 'GPOS'.
     */
    std::string features;

    /** The metrics of a font.
     *
     * @note: unit is 'em'.
     */
    hi::font_metrics metrics;

    /** List of fonts to use as a fallback for this font.
     */
    std::vector<hi::font *> fallback_chain;

    font() = default;
    virtual ~font() = default;
    font(font const&) = delete;
    font& operator=(font const&) = delete;
    font(font&&) = delete;
    font& operator=(font&&) = delete;

    /** Return if the font is loaded.
     *
     * @return true if the font is fully loaded, false if only metadata of the font is available.
     */
    [[nodiscard]] virtual bool loaded() const noexcept = 0;

    /** Get the glyph for a code-point.
     * @return glyph-id, or invalid when not found or error.
     */
    [[nodiscard]] glyph_id find_glyph(char32_t c) const noexcept
    {
        return char_map.find(c);
    }

    /** Get the glyphs for a grapheme.
     * @return a set of glyph-ids, or invalid when not found or error.
     */
    [[nodiscard]] lean_vector<glyph_id> find_glyph(grapheme g) const
    {
        // Create a glyph_ids object for a single grapheme.
        auto r = lean_vector<glyph_id>{};

        // First try composed normalization
        for (hilet c : g.composed()) {
            if (hilet glyph_id = find_glyph(c)) {
                r.push_back(glyph_id);
            } else {
                r.clear();
                break;
            }
        }

        if (r.empty()) {
            // Now try decomposed normalization
            for (hilet c : g.decomposed()) {
                if (hilet glyph_id = find_glyph(c)) {
                    r.push_back(glyph_id);
                } else {
                    r.clear();
                    break;
                }
            }
        }

        return r;
    }

    /** Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * @param glyph_id the id of a glyph inside the font.
     * @return The path loaded from the font file.
     * @throws std::exception If there was an error while loading the path.
     *         Recommend to disable the font on error.
     */
    [[nodiscard]] virtual graphic_path get_path(hi::glyph_id glyph_id) const = 0;

    /** Get the advance for a glyph.
     *
     * @param glyph_id The glyph to look up the advance for.
     * @return The advance for the glyph.
     * @throws std::exception If there was an error looking up the glyph.
     */
    [[nodiscard]] virtual float get_advance(hi::glyph_id glyph_id) const = 0;

    /** Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * @param glyph_id the id of a glyph inside the font.
     * @param metrics The metrics constructed by the loader.
     * @param lookahead_glyph_id The id of a glyph to the right, needed for kerning.
     * @return true on success, false on error.
     */
    [[nodiscard]] virtual glyph_metrics get_metrics(hi::glyph_id glyph_id) const = 0;

    struct shape_run_result_type {
        /** The horizontal advance of each grapheme.
         */
        std::vector<float> advances;

        /** The number of glyphs used by each grapheme.
         */
        std::vector<size_t> glyph_count;

        /** The glyphs representing all the graphemes.
         *
         * There may be zero or more glyphs for each grapheme.
         * The difference may be due to having to add accent-glyphs
         * or merging glyphs into ligatures.
         */
        std::vector<glyph_id> glyphs;

        /** Position of each glyph, relative to the grapheme
         *
         * There is exactly one position for each glyph.
         */
        std::vector<point2> glyph_positions;

        /** The bounding rectangle for each glyph, relative to glyph_position.
         *
         * There is exactly one bounding rectangle for each glyph.
         */
        std::vector<aarectangle> glyph_rectangles;

        void reserve(size_t count) noexcept
        {
            advances.reserve(count);
            glyph_count.reserve(count);
            glyphs.reserve(count);
            glyph_positions.reserve(count);
            glyph_rectangles.reserve(count);
        }

        /** Scale and position the result of the run.
         *
         * @param s The font-size to scale by
         * @param x The horizontal position, calculated after scaling
         */
        void scale_and_offset(float s) noexcept
        {
            auto S = scale2{s};

            for (auto& tmp : advances) {
                tmp = s * tmp;
            }
            for (auto& tmp : glyph_positions) {
                tmp = S * tmp;
            }
            for (auto& tmp : glyph_rectangles) {
                tmp = S * tmp;
            }
        }
    };

    /** Shape a run of graphemes.
     *
     * A run of graphemes is a piece of text that is:
     * - from the same style,
     * - from the same font,
     * - from the same language and script, and
     * - on the same line.
     *
     * A run needs to be shaped by the font-file itself as it handles:
     * - language/script depended glyph substitution for ligatures, accents and cursive text.
     * - language/script depended glyph positioning for kerning, accents and cursive text.
     *
     * @param language The language of this run of graphemes.
     * @param script The script of this run of graphemes.
     * @param run The run of graphemes.
     * @return The glyphs and coordinates to display, and coordinates of grapheme for interaction.
     */
    [[nodiscard]] virtual shape_run_result_type shape_run(iso_639 language, iso_15924 script, gstring run) const = 0;

    glyph_atlas_info& atlas_info(glyph_id glyph) const
    {
        if (*glyph >= _glyph_atlas_table.size()) [[unlikely]] {
            _glyph_atlas_table.resize(*glyph + 1);
        }

        hi_axiom_bounds(*glyph, _glyph_atlas_table);
        return _glyph_atlas_table[*glyph];
    }

    [[nodiscard]] font_variant font_variant() const noexcept
    {
        return {weight, style};
    }

    [[nodiscard]] friend std::string to_string(font const& rhs) noexcept
    {
        return std::format(
            "{} - {}: style={}{}{}{}{}{}, features={}",
            rhs.family_name,
            rhs.sub_family_name,
            rhs.monospace ? 'M' : '_',
            rhs.serif ? 'S' : '_',
            rhs.style == font_style::italic ? 'I' : '_',
            rhs.condensed ? 'C' : '_',
            to_char(rhs.weight),
            rhs.optical_size,
            rhs.features);
    }

private:
    mutable std::vector<glyph_atlas_info> _glyph_atlas_table;
};

} // namespace hi::inline v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
export template<>
struct std::formatter<hi::font, char> : formatter<std::string_view, char> {
    auto format(hi::font const& t, auto& fc) const
    {
        return formatter<string_view, char>::format(to_string(t), fc);
    }
};
