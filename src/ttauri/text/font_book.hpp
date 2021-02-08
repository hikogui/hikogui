// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/text/font_description.hpp"
#include "ttauri/text/grapheme.hpp"
#include "ttauri/text/font.hpp"
#include "ttauri/text/font_family_id.hpp"
#include "ttauri/text/font_id.hpp"
#include "ttauri/text/font_grapheme_id.hpp"
#include "ttauri/text/font_glyph_ids.hpp"
#include "ttauri/URL.hpp"
#include "ttauri/alignment.hpp"
#include <limits>
#include <array>
#include <new>


namespace tt {

/** font_book keeps track of multiple fonts.
 * The font_book is instantiated during application startup
 * and is available through Foundation_globals->font_book.
 *
 * 
 */
class font_book {
public:
    static inline std::unique_ptr<font_book> global;

    font_book(std::vector<URL> const &font_directories);

    /** Register a font.
     * Duplicate registrations will be ignored.
     * 
     * When a font file is registered the file will be temporarily opened to read and cache a set of properties:
     *  - The English font Family from the 'name' table.
     *  - The weight, width, slant & design-size from the 'fdsc' table.
     *  - The character map 'cmap' table.
     *
     * @param url Location of font.
     * @param post_process Calculate font fallback
     */
    font_id register_font(URL url, bool post_process=true);

    /** Post process font_book
     * Should be called after a set of register_font() calls
     * This calculates font fallbacks.
     */
    void post_process() noexcept;

    /** Find font family id.
     * This function will always return a valid font_family_id by walking the fallback-chain.
     */
    [[nodiscard]] font_family_id find_family(std::string_view family_name) const noexcept;

    /** Register font family id.
     * If the family already exists the existing family_id is returned.
     */
    [[nodiscard]] font_family_id register_family(std::string_view family_name) noexcept;

    /** Find a font closest to the variant.
     * This function will always return a valid font_id.
     *
     * @param family_id a valid family id.
     * @param variant The variant of the font to select.
     * @return a valid font id.
     */
    [[nodiscard]] font_id find_font(font_family_id family_id, font_variant variant) const noexcept;

    /** Find a font closest to the variant.
    * This function will always return a valid font_id.
    *
    * @param family_id a valid family id.
    * @param weight The weight of the font to select.
    * @param italic If the font to select should be italic or not.
    * @return a valid font id.
    */
    [[nodiscard]] font_id find_font(font_family_id family_id, font_weight weight, bool italic) const noexcept;

    /** Find a font closest to the variant.
     * This function will always return a valid font_id.
     *
     * @param family_name A name of a font family, which may be invalid.
     * @param weight The weight of the font to select.
     * @param italic If the font to select should be italic or not.
     * @return a font id, possibly from a fallback font.
     */
    [[nodiscard]] font_id find_font(std::string_view family_name, font_weight weight, bool italic) const noexcept;

    [[nodiscard]] font const &get_font(font_id font_id) const noexcept;

    /** Find a glyph using the given code-point.
     * This function will find a glyph matching the grapheme in the selected font, or
     * find the glyph in the fallback font.
     *
     * @param font_id The font to use to find the grapheme in.
     * @param grapheme The Unicode grapheme to find in the font.
     * @return A list of glyphs which matched the grapheme.
     */
    [[nodiscard]] font_glyph_ids find_glyph(font_id font_id, grapheme grapheme) const noexcept;

private:
    struct fontEntry {
        URL url;
        font_description description;
        mutable std::unique_ptr<font> font;
        std::vector<font_id> fallbacks;

        fontEntry(URL url, font_description description) noexcept :
            url(std::move(url)), description(std::move(description)), font(), fallbacks()
        {
        }
    };

    /** Table of font_family_ids index using the family-name.
     */
    std::unordered_map<std::string, font_family_id> family_names;

    /** A list of family name -> fallback family name
     */
    std::unordered_map<std::string, std::string> family_name_fallback_chain;

    /** Different fonts; variants of a family.
     */
    std::vector<std::array<font_id, font_variant::max()>> font_variants;

    std::vector<fontEntry> font_entries;

    /** Same as family_name, but will also have resolved font families from the fallback_chain.
     * Must be cleared when a new font family is registered.
     */
    mutable std::unordered_map<std::string, font_family_id> family_name_cache;

    /**
     * Must be cleared when a new font is registered.
     */
    mutable std::unordered_map<font_grapheme_id, font_glyph_ids> glyph_cache;
    void calculate_fallback_fonts(fontEntry &entry, std::function<bool(font_description const&,font_description const&)> predicate) noexcept;

    /** Find the glyph for this specific font.
     * This will open the font file if needed.
     */
    [[nodiscard]] font_glyph_ids find_glyph_actual(font_id font_id, grapheme grapheme) const noexcept;

    /** Morph the set of glyphs using the font's morph tables.
     */
    //void morph_glyphs(glyph_array &glyphs) const noexcept;

    /** Lookup the glyphs in the atlas and optionally render.
    */
    //void atlas_lookup(glyph &glyph) noexcept;

    /** Lookup the glyphs in the atlas and optionally render.
     */
    //void atlas_lookup(glyph_array &glyphs) noexcept;

    //void kern_glyphs(glyph_array &glyphs) const noexcept;

    /** Find a fallback font family name
    * Repeated calls will follow the chain.
    */
    [[nodiscard]] std::string const &find_fallback_family_name(std::string const &name) const noexcept;

    void create_family_name_fallback_chain() noexcept;
};

}
