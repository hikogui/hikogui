// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_metrics.hpp"
#include "glyph_atlas_info.hpp"
#include "glyph_ids.hpp"
#include "font_weight.hpp"
#include "font_variant.hpp"
#include "font_metrics.hpp"
#include "font_char_map.hpp"
#include "../unicode/unicode_mask.hpp"
#include "../unicode/gstring.hpp"
#include "../i18n/iso_15924.hpp"
#include "../i18n/iso_639.hpp"
#include "../graphic_path.hpp"
#include "../utility/module.hpp"
#include "../hash_map.hpp"
#include "../lean_vector.hpp"
#include <span>
#include <vector>
#include <map>
#include <string>

namespace hi::inline v1 {

/*! A font.
 * This class has information on how to shape text and
 * get glyphs consisting of bezier contours.
 */
class font {
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
    bool italic = false;
    bool condensed = false;
    font_weight weight = font_weight::Regular;
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
    [[nodiscard]] lean_vector<glyph_id> find_glyph(grapheme g) const;

    /** Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * @param glyph_id the id of a glyph inside the font.
     * @return The path loaded from the font file.
     * @throws std::exception If there was an error while loading the path.
     *         Recommend to disable the font on error.
     */
    [[nodiscard]] virtual graphic_path load_path(hi::glyph_id glyph_id) const = 0;

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * \param glyph_id the id of a glyph inside the font.
     * \param metrics The metrics constructed by the loader.
     * \param lookahead_glyph_id The id of a glyph to the right, needed for kerning.
     * \return true on success, false on error.
     */
    [[nodiscard]] virtual glyph_metrics load_metrics(hi::glyph_id glyph_id) const = 0;

    /** Get the kerning between two glyphs.
     *
     * @param current_glyph The glyph on the left
     * @param next_glyph The glyph on the right
     * @return The vector to add to the advance of the current_glyph.
     */
    [[nodiscard]] virtual vector2 get_kerning(hi::glyph_id current_glyph, hi::glyph_id next_glyph) const = 0;

    struct shape_run_result_type {
        /** The bounding rectangle for each grapheme.
         *
         * The coordinates are in EM units and start at zero
         * at the left-most / first grapheme.
         *
         * There is exactly one entry for each grapheme in the call to `shape_run()`.
         *
         * These bounding rectangles are used for user-interacting
         * with the graphemes.
         */
        std::vector<aarectangle> grapheme_boxes;

        /** The glyphs representing all the graphemes.
         *
         * There may be zero or more glyphs for each grapheme.
         * The difference may be due to having to add accent-glyphs
         * or merging glyphs into ligatures.
         */
        std::vector<glyph_id> glyphs;

        /** The bounding rectangle for each glyph.
         *
         * The coordinates are in EM units and start at zero
         * at the left-most / first grapheme.
         *
         * There is exactly one bounding rectangle for each glyph.
         */
        std::vector<aarectangle> glyph_boxes;

        /** The advance for this run of graphemes.
         *
         * The value is in EM units.
         */
        float advance;
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

    glyph_atlas_info& atlas_info(glyph_ids const& glyphs) const
    {
        if (glyphs.has_num_glyphs<1>()) [[likely]] {
            hilet index = static_cast<std::size_t>(get<0>(glyphs));
            if (index >= _single_glyph_atlas_table.size()) [[unlikely]] {
                _single_glyph_atlas_table.resize(index + 1);
            }
            return _single_glyph_atlas_table[index];

        } else {
            return _multi_glyph_atlas_table[glyphs];
        }
    }

    [[nodiscard]] font_variant font_variant() const noexcept
    {
        return {weight, italic};
    }

    [[nodiscard]] friend std::string to_string(font const& rhs) noexcept
    {
        return std::format(
            "{} - {}: style={}{}{}{}{}{}, features={}",
            rhs.family_name,
            rhs.sub_family_name,
            rhs.monospace ? 'M' : '_',
            rhs.serif ? 'S' : '_',
            rhs.italic ? 'I' : '_',
            rhs.condensed ? 'C' : '_',
            to_char(rhs.weight),
            rhs.optical_size,
            rhs.features);
    }

private:
    mutable std::vector<glyph_atlas_info> _single_glyph_atlas_table;
    mutable hash_map<glyph_ids, glyph_atlas_info> _multi_glyph_atlas_table;
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::font, CharT> : formatter<std::string_view, CharT> {
    auto format(hi::font const& t, auto& fc)
    {
        return formatter<string_view, CharT>::format(to_string(t), fc);
    }
};
