// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/grapheme.hpp"
#include "TTauri/Foundation/text.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include <limits>
#include <array>
#include <new>


namespace TTauri {



struct attributed_grapheme {
    font_style style;
    uint32_t text_index;
    grapheme grapheme;
};

/** Intermediate representation of a glyph, before text-shaping.
 */
struct attributed_glyph {
    font_style style;
    // Upper 4 bits on the text_index contains the number graphemes that are merged in a single glyph.
    // Or zero when the glyph combines with the previous glyph.
    uint32_t text_index_and_size;

    uint16_t font_id;
    uint16_t glyph_id;
};

/** All information for a glyph after text-shaping.
 */
struct placed_glyph {

};

/**
 * 128-bits.
 */
struct glyph {
    font_id font_id;
    glyph_id glyph_id;

    uint16_t grapheme_index; ///< Index back into the text.
    glyph_atlas_id atlas_id; ///< Index into to atlas image.

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

class FontBook {
    /** Register a font.
     * Duplicate registrations will be ignored.
     * 
     * When a font file is registered the file will be temporarily opened to read and cache a set of properties:
     *  - The English Font Family from the 'name' table.
     *  - The weight, width, slant & design-size from the 'fdsc' table.
     *  - The character map 'cmap' table.
     */
    void register_font(URL font_file);

    /** Register a font-style for use when shaping text.
     * The font-style keeps a cache for which specific glyph to use for a set of code-points.
     * Duplicate registrations will return the same font_style_id.
     */
    [[nodiscard]] font_style_id register_style(std::string font_family, uint8_t size, int weight, bool condensed, bool italic, uint8_t color) noexcept;

    /** Find font super family id.
     * Automatic fall-back to the "Noto" super font family.
     */
    [[nodiscard]] int find_super_family_id(std::string const &name) const noexcept;

    /** Find a glyph using the given code-point.
     * This function will find a glyph for a font that looks closest to the given style.
     *
     * @param style The font style to use for finding glyphs.
     * @param codePoint the Unicode code-point to find in the font.
     * @return The font/glyph containing the code point, or boolean false if not found.
     */
    [[nodiscard]] glyph find_glyph(font_style_id style, char32_t codePoint) const noexcept;

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
    [[nodiscard]] glyph_array find_glyph(font_style_id style, grapheme g) const noexcept;

    /** Find all the glyphs in a text.
     * The text a set of graphemes which is decorated with font_style_ids.
     * The result is a glyph array including position and size on where to draw each glyph on the screen.
     */
    [[nodiscard]] glyph_array shape_text(text text, float max_width) const noexcept;

    [[nodiscard]] glyph_metrics get_glyph_metrics(glyph id) noexcept;

    [[nodiscard]] path get_glyph_path(glyph id) noexcept;

private:

    /** Load the font.
     * We have already read the font once; if opening the font fails now we can't handle the errors.
     */
    void load_font(font_id id) const noexcept;

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
};


}
