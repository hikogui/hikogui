// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/grapheme.hpp"
#include "TTauri/Foundation/text.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/FontDescription.hpp"
#include "TTauri/Foundation/tagged_id.hpp"
#include <limits>
#include <array>
#include <new>


namespace TTauri {



//struct attributed_grapheme {
//    font_style style;
//    uint32_t text_index;
//    grapheme grapheme;
//};

/** Intermediate representation of a glyph, before text-shaping.
 */
//struct attributed_glyph {
//    font_style style;
//    // Upper 4 bits on the text_index contains the number graphemes that are merged in a single glyph.
//    // Or zero when the glyph combines with the previous glyph.
//    uint32_t text_index_and_size/;

//    uint16_t font_id;
//    uint16_t glyph_id;
//};

/** All information for a glyph after text-shaping.
 */
struct placed_glyph {

};

/**
 * 128-bits.
 */
struct glyph {
    uint16_t font_id;
    uint16_t glyph_id;

    uint16_t grapheme_index; ///< Index back into the text.
    //glyph_atlas_id atlas_id; ///< Index into to atlas image.

    uint8_t width; ///< width in pixels of the atlas sub-image
    uint8_t height; ///< height in pixels of the atlas sub-image
    int8_t offset_x; ///< offset in pixels of the origin inside the atlas sub-image
    int8_t offset_y; ///< offset in pixels of the origin inside the atlas sub-image

    int8_t advance_x; ///< advance x in 1/3pt, this allows for LCD sub-pixel positioning on low resolution displays.
    int8_t advance_y; ///< advance y in 1/3pt, this allows for LCD sub-pixel positioning on low resolution displays.
    uint8_t font_size; ///< size to render font at in pt. The atlas was rendered at 24pt.

    /** Indicated color of the glyph, and line/alignment.
     * - [7] Start-of-line
     * - [6] End-of-line
     * - [5:4] 0 = Align-left, 1 = Align-right, 2 = Align-center, 3 = Justify
     * - [3:0] Color index.
     */
    uint8_t text_color_and_alignment;

    operator bool () const noexcept {
        return static_cast<bool>(font_id);
    }
};

using glyph_array = std::vector<glyph>;






using FontFamilyID = tagged_id<uint16_t, class FontBook, "fontfamily_id"_tag>;
using FontID = tagged_id<uint16_t, class FontBook, "font_id"_tag>;
class FontBook {
    /** Table of FontFamilyIDs index using the family-name.
     */
    std::unordered_map<std::string,FontFamilyID> family_name_table;

    /** Same as family_name_table, but will also have resolved font families from the fallback_chain.
     */
    mutable std::unordered_map<std::string,FontFamilyID> family_name_cache;

    /**
     */
    std::unordered_map<std::string,std::string> family_name_fallback_chain;


    /** FontID indexed using FontFamilyID + font_variant.
     */
    std::vector<std::array<FontID,FontVariant::max()>> font_variant_table;

    /** Descriptions of fonts, indexed using FontID.
     */
    std::vector<FontDescription> font_descriptions;

    /** If no matching font is found, lookup glyphs in one of the following fonts.
     */
    std::vector<FontID> last_resort_font_table;

public:
    FontBook(std::vector<URL> const &font_directories);

    /** Register a font.
     * Duplicate registrations will be ignored.
     * 
     * When a font file is registered the file will be temporarily opened to read and cache a set of properties:
     *  - The English Font Family from the 'name' table.
     *  - The weight, width, slant & design-size from the 'fdsc' table.
     *  - The character map 'cmap' table.
     */
    FontID register_font(URL font_file);

    /** Find font family id.
     * This function will always return a valid FontFamilyID by walking the fallback-chain.
     */
    [[nodiscard]] FontFamilyID find_family(std::string_view family_name) const noexcept;

    /** Register font family id.
     * If the family already exists the existing family_id is returned.
     */
    [[nodiscard]] FontFamilyID register_family(std::string_view family_name) noexcept;

    /** Find a font closest to the variant.
     * This function will always return a valid FontID.
     *
     * @param family_id a valid family id.
     * @param variant The variant of the font to select.
     * @return a valid font id.
     */
    [[nodiscard]] FontID find_font(FontFamilyID family_id, FontVariant variant) const noexcept;

    /** Find a font closest to the variant.
    * This function will always return a valid FontID.
    *
    * @param family_id a valid family id.
    * @param weight The weight of the font to select.
    * @param italic If the font to select should be italic or not.
    * @return a valid font id.
    */
    [[nodiscard]] FontID find_font(FontFamilyID family_id, font_weight weight, bool italic) const noexcept;

    /** Find a font closest to the variant.
     * This function will always return a valid FontID.
     *
     * @param family_id a valid family id.
     * @param weight The weight of the font to select.
     * @param italic If the font to select should be italic or not.
     * @return a valid font id.
     */
    [[nodiscard]] FontID find_font(std::string_view family_name, font_weight weight, bool italic) const noexcept;

    /** Find a glyph using the given code-point.
     * This function will find a glyph for a font that looks closest to the given style.
     *
     * @param style The font style to use for finding glyphs.
     * @param codePoint the Unicode code-point to find in the font.
     * @return The font/glyph containing the code point, or boolean false if not found.
     */
    [[nodiscard]] glyph find_glyph(int style, char32_t codePoint) const noexcept;

    /** Find a glyph using the given code-point.
    * This function is used for finding combining marks in the same font as the
    * primary code-point in a grapheme.
    *
    * @param primaryGlyph The primary glyph to add the combining marks to.
    * @param codePoint the Unicode code-point to find in the font.
    * @return The font/glyph containing the code point, or boolean false if not found.
    */
    [[nodiscard]] glyph find_glyph(glyph primary_glyph, char32_t codePoint) const noexcept;

    /** Find a set of glyphs matching the given grapheme.
     * If the grapheme results in combining marks all glyphs will be from the same font.
     */
    [[nodiscard]] glyph_array find_glyph(int style, grapheme g) const noexcept;

    /** Find all the glyphs in a text.
     * The text a set of graphemes which is decorated with font_style_ids.
     * The result is a glyph array including position and size on where to draw each glyph on the screen.
     */
    [[nodiscard]] glyph_array shape_text(text text, float max_width) const noexcept;

    //[[nodiscard]] glyph_metrics get_glyph_metrics(glyph id) noexcept;

    //[[nodiscard]] path get_glyph_path(glyph id) noexcept;

private:

    /** Load the font.
     * We have already read the font once; if opening the font fails now we can't handle the errors.
     */
    void load_font(int id) const noexcept;

    /** Morph the set of glyphs using the font's morph tables.
     */
    void morph_glyphs(glyph_array &glyphs) const noexcept;

    /** Lookup the glyphs in the atlas and optionally render.
    */
    void atlas_lookup(glyph &glyph) noexcept;

    /** Lookup the glyphs in the atlas and optionally render.
     */
    void atlas_lookup(glyph_array &glyphs) noexcept;

    void kern_glyphs(glyph_array &glyphs) const noexcept;

    /** Find a fallback font family name
    * Repeated calls will follow the chain.
    */
    [[nodiscard]] std::string const &FontBook::find_fallback_family_name(std::string const &name) const noexcept;

    void create_family_name_fallback_chain() noexcept;

};


}
