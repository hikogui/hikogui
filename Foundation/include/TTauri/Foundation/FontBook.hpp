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

/** Describes how a grapheme should be underlined when rendering the text.
 * It is carried with the grapheme and glyphs, so that the text render engine
 * can draw the decoration after the text is shaped and in rendering-order
 * (left to right) and, this makes it easier to correctly render the decoration
 * of multiple glyphs in a single stroke.
 */
enum class font_underline {
    None,
    Underlined,
    Striketrough,
    WavyLine,
};

/** Describes how the background of a grapheme should drawn when rendering the text.
* It is carried with the grapheme and glyphs, so that the text render engine
* can draw the decoration after the text is shaped and in rendering-order
* (left to right) and, this makes it easier to correctly render the decoration
* of multiple glyphs in a single stroke.
*/
enum class font_background {
    None,
    Selected,
    SearchMatch,
    Reserved3,
};

/** The font-style carries all the information needed to draw a grapheme in a certain style.
 * The font-style is compressed in a single 32-bit integer to improve speed of comparison in
 * the style table and for reduced storage when it is accompaniment with a grapheme or glyph.
 */
class font_style {
    uint32_t value;

    constexpr int super_family_bits = 8;
    constexpr int serif_bits = 1;
    constexpr int monospace_bits = 1;
    constexpr int italic_bits = 1;
    constexpr int condensed_bits = 1;
    constexpr int weight_bits = 4;
    constexpr int size_bits = 8;
    constexpr int color_bits = 4;
    constexpr int decoration_bits = 2;
    constexpr int background_bits = 2;

    constexpr int super_family_shift = (sizeof(value) * CHAR_BIT) - super_family_bits;
    constexpr int serif_shift = super_family_shift - serif_bits;
    constexpr int monospace_shift = serif_shift - monospace_bits;
    constexpr int italic_shift = monospace_shift - italic_bits;
    constexpr int condensed_shift = italic_shift - condensed_bits;
    constexpr int weight_shift = condensed_shift - weight_bits;
    constexpr int size_shift = weight_shift - size_bits;
    constexpr int color_shift = size_shift - color_bits;
    constexpr int underline_shift = color_shift - underline_bits;
    constexpr int background_shift = underline_shift - background_bits;
    static_assert(background_shift >= 0);

    constexpr uint64_t super_family_mask = (1ULL << super_family_bits) - 1;
    constexpr uint64_t serif_mask = (1ULL << serif_bits) - 1;
    constexpr uint64_t monospace_mask = (1ULL << monospace_bits) - 1;
    constexpr uint64_t italic_mask = (1ULL << italic_bits) - 1;
    constexpr uint64_t condensed_mask = (1ULL << condensed_bits) - 1;
    constexpr uint64_t weight_mask = (1ULL << weight_bits) - 1;
    constexpr uint64_t size_mask = (1ULL << size_bits) - 1;
    constexpr uint64_t color_mask = (1ULL << color_bits) - 1;
    constexpr uint64_t underline_mask = (1ULL << underline_bits) - 1;
    constexpr uint64_t background_mask = (1ULL << background_bits) - 1;

public:
    /** Return the font super-family id.
     * The super font are the combination of fonts of multiple-styles
     * including those of serif/sans-serif, mono and slab types
     * weights and display sizes.
     */
    [[nodiscard]] uint8_t super_family_id {
        return (value >> super_family_shift) & super_family_mask;
    }

    [[nodiscard]] bool serif() const noexcept { return ((value >> serif_shift) & serif_mask) != 0; }
    [[nodiscard]] bool monospace() const noexcept { return ((value >> monospace_shift) & monospace_mask) != 0; }
    [[nodiscard]] bool italic() const noexcept { return ((value >> italic_shift) & italic_mask) != 0; }
    [[nodiscard]] bool condensed() const noexcept { return ((value >> condensed_shift) & condensed_mask) != 0; }

    /** Font weight.
     * A multiplier between 0.0 and 2.0 representing the weight of character:
     * 
     * value | code | name
     * -----:| ----:|:------
     *     1 |  100 | Thin / Hairline
     *     2 |  200 | Ultra-light / Extra-light
     *     3 |  300 | Light
     *     4 |  400 | Normal / Regular
     *     5 |  500 | Medium
     *     6 |  600 | Semi-bold / Demi-bold
     *     7 |  700 | Bold
     *     8 |  800 | Extra-bold / Ultra-bold
     *     9 |  900 | Heavy / Black
     *    10 | 1000 | Extra-black / Ultra-black
     */
    [[nodiscard]] int weight() const noexcept {
        return (value >> weight_shift) & weight_mask;
    }

    /** Size of text in pt.
     */
    [[nodiscard]] float size() const noexcept { return (value >> size_shift) & size_mask; }

    /** Text color index.
     * Value between 0-15.
     */
    [[nodiscard]] int color() const noexcept { return (value >> color_shift) & color_mask; }

    [[nodiscard]] font_underline underline() const noexcept { return static_cast<font_underline>((value >> underline_shift) & underline_mask); }
    
    [[nodiscard]] font_background background() const noexcept { return static_cast<font_background>((value >> background_shift) & background_mask); }
};

struct attributed_grapheme {
    font_style style;
    uint32_t text_index;
    grapheme grapheme;

    // XXX Language code?

};

/** Intermediate representation of a glyph, before text-shaping.
 */
struct attributed_glyph {
    font_style style;
    // Upper 4 bits on the text_index contains the number graphemes that are merged in a single glyph.
    // Or zero when the glyph combines with the previous glyph.
    uint32_t text_index_and_size;

    // XXX Language code?


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