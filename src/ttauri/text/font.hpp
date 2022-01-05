// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_metrics.hpp"
#include "glyph_atlas_info.hpp"
#include "glyph_ids.hpp"
#include "gstring.hpp"
#include "unicode_mask.hpp"
#include "font_weight.hpp"
#include "font_variant.hpp"
#include "font_metrics.hpp"
#include "../graphic_path.hpp"
#include "../resource_view.hpp"
#include "../exception.hpp"
#include "../required.hpp"
#include "../URL.hpp"
#include "../hash_map.hpp"
#include <span>
#include <vector>
#include <map>
#include <string>

namespace tt::inline v1 {

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

    tt::unicode_mask unicode_mask;

    /** The metrics of a font.
     *
     * @note: unit is 'em'.
     */
    tt::font_metrics metrics;

    /** List of fonts to use as a fallback for this font.
     */
    std::vector<tt::font *> fallback_chain;

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
    [[nodiscard]] virtual tt::glyph_id find_glyph(char32_t c) const noexcept = 0;

    /** Get the glyphs for a grapheme.
     * @return a set of glyph-ids, or invalid when not found or error.
     */
    [[nodiscard]] tt::glyph_ids find_glyph(grapheme g) const noexcept;

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * \param glyph_id the id of a glyph inside the font.
     * \param path The path constructed by the loader.
     * \return empty on failure, or the glyphID of the metrics to use.
     */
    virtual std::optional<tt::glyph_id> load_glyph(tt::glyph_id glyph_id, graphic_path &path) const noexcept = 0;

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * \param glyph_id the id of a glyph inside the font.
     * \param metrics The metrics constructed by the loader.
     * \param lookahead_glyph_id The id of a glyph to the right, needed for kerning.
     * \return true on success, false on error.
     */
    virtual bool load_glyph_metrics(
        tt::glyph_id glyph_id,
        glyph_metrics &metrics,
        tt::glyph_id lookahead_glyph_id = tt::glyph_id{}) const noexcept = 0;

    /** Get the kerning between two glyphs.
    * 
    * @param current_glyph The glyph on the left
    * @param next_glyph The glyph on the right
    * @return The vector to add to the advance of the current_glyph.
    */
    [[nodiscard]] virtual vector2 get_kerning(tt::glyph_id current_glyph, tt::glyph_id next_glyph) const noexcept = 0;

    glyph_atlas_info &atlas_info(glyph_ids const &glyphs) const noexcept
    {
        if (glyphs.has_num_glyphs<1>()) [[likely]] {
            ttlet index = static_cast<std::size_t>(get<0>(glyphs));
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
            "{} - {}: {}{}{}{}{} {} num-code-points={}",
            rhs.family_name,
            rhs.sub_family_name,
            rhs.monospace ? 'M' : '_',
            rhs.serif ? 'S' : '_',
            rhs.italic ? 'I' : '_',
            rhs.condensed ? 'C' : '_',
            to_char(rhs.weight),
            rhs.optical_size,
            rhs.unicode_mask.size());
    }

private:
    mutable std::vector<glyph_atlas_info> _single_glyph_atlas_table;
    mutable hash_map<glyph_ids, glyph_atlas_info> _multi_glyph_atlas_table;
};

} // namespace tt::inline v1

template<typename CharT>
struct std::formatter<tt::font, CharT> : formatter<std::string_view, CharT> {
    auto format(tt::font const &t, auto &fc)
    {
        return formatter<string_view, CharT>::format(to_string(t), fc);
    }
};
