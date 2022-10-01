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
#include "../unicode/unicode_mask.hpp"
#include "../unicode/gstring.hpp"
#include "../i18n/iso_15924.hpp"
#include "../i18n/iso_639.hpp"
#include "../graphic_path.hpp"
#include "../exception.hpp"
#include "../utility.hpp"
#include "../hash_map.hpp"
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

    /** A string representing the features of a font.
     * This will be a comma separated list of features, mostly tables like 'kern' and 'GPOS'.
     */
    std::string features;

    hi::unicode_mask unicode_mask;

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
    font(font const &) = delete;
    font &operator=(font const &) = delete;
    font(font &&) = delete;
    font &operator=(font &&) = delete;

    /** Return if the font is loaded.
     *
     * @return true if the font is fully loaded, false if only metadata of the font is available.
     */
    [[nodiscard]] virtual bool loaded() const noexcept = 0;

    /** Get the glyph for a code-point.
     * @return glyph-id, or invalid when not found or error.
     */
    [[nodiscard]] virtual hi::glyph_id find_glyph(char32_t c) const noexcept = 0;

    /** Get the glyphs for a grapheme.
     * @return a set of glyph-ids, or invalid when not found or error.
     */
    [[nodiscard]] hi::glyph_ids find_glyph(grapheme g) const noexcept;

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * \param glyph_id the id of a glyph inside the font.
     * \param path The path constructed by the loader.
     * \return empty on failure, or the glyphID of the metrics to use.
     */
    virtual std::optional<hi::glyph_id> load_glyph(hi::glyph_id glyph_id, graphic_path &path) const noexcept = 0;

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * \param glyph_id the id of a glyph inside the font.
     * \param metrics The metrics constructed by the loader.
     * \param lookahead_glyph_id The id of a glyph to the right, needed for kerning.
     * \return true on success, false on error.
     */
    virtual bool load_glyph_metrics(
        hi::glyph_id glyph_id,
        glyph_metrics &metrics,
        hi::glyph_id lookahead_glyph_id = hi::glyph_id{}) const noexcept = 0;

    /** Get the kerning between two glyphs.
     *
     * @param current_glyph The glyph on the left
     * @param next_glyph The glyph on the right
     * @return The vector to add to the advance of the current_glyph.
     */
    [[nodiscard]] virtual vector2 get_kerning(hi::glyph_id current_glyph, hi::glyph_id next_glyph) const noexcept = 0;

    struct substitution_and_kerning_type {
        /** The glyph.
         *
         * On input: the original glyph
         * On output: the substituted glyph, possibly a ligature glyph. Or empty if this glyph was substituted
         *            by a previous ligature.
         */
        glyph_id glyph;

        /** The advance in font-unit coordinate system.
         *
         * On input: the original advance for the glyph
         * On output: the advance adjusted by kerning, or the partial advance of the character within
         *            a ligature. All advances of a ligature added together will be the total advance of
         *            the full ligature including kerning.
         */
        vector2 advance;
    };

    /** Substitute and kern a run of glyphs.
     *
     * @param language The language that the word is written in.
     * @param script The script that the word is written in.
     * @param[in,out] word A run of glyphs, from the same font, font-size and script of a word.
     */
    virtual void substitution_and_kerning(iso_639 language, iso_15924 script, std::vector<substitution_and_kerning_type> &word)
        const noexcept = 0;

    glyph_atlas_info &atlas_info(glyph_ids const &glyphs) const noexcept
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

    [[nodiscard]] friend std::string to_string(font const &rhs) noexcept
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
    auto format(hi::font const &t, auto &fc)
    {
        return formatter<string_view, CharT>::format(to_string(t), fc);
    }
};
