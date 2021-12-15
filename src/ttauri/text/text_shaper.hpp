// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../alignment.hpp"
#include "grapheme.hpp"
#include "text_style.hpp"
#include "glyph_ids.hpp"
#include <vector>

#pragma once

namespace tt::inline v1 {

/** Text shaper.
 *
 * This class takes text as a set of graphemes attributed with font, size, style and color.
 *
 * Steps:
 *  1. Load default glyphs and metrics scaled to the font-size of each glyph.
 *  2. Fold default glyphs to a certain width by inserting line-separators.
 *  3. Run unicode bidirectional algorithm.
 *  4. Reload glyphs and metrics of any brackets.
 *  5. Morph glyphs.
 *  6. Position glyphs including kerning and justifcation.
 *
 */
class text_shaper {
public:
    /** Construct a text_shaper with a text and alignment.
     *
     * Horizontal alignment is done for each line independed of the writing direction.
     * This allows labels to remain aligned in the same direction on the user-interface
     * even when the labels have translations in different languages.
     *
     * Label widgets should flip the alignement passed to the text shaper when the
     * user interface is mirrored.
     *
     * Text edit fields may want to change the alignment of the text depending on the
     * dominant writing direction, for more natural typing.
     *
     * Vertical alignment of the text determines what y=0 means:
     *  - top: y = 0 is the base-line of the first line, all other lines are at y < 0.
     *  - bottom: y = 0 is the base-line of the last line, all other lines are at y > 0.
     *  - middle, odd: y = 0 is the base-line of the middle line.
     *  - middle, even: y = 0 is half way between the base-lines of the middle two lines.
     *
     * @param text The text as a vector of attributed graphemes.
     *             Use U+2029 as paragraph separator, and if needed U+2028 as line separator.
     * @param vertical_alignment How the text will be aligned vertically.
     * @param line_spacing A multiplier to scale the distance between lines compared to the
     *                     natural line spacing of the font: ascender + descender + line-gap.
     * @param paragraph_spacing A multiplier to scale the distance between lines compared to the
     *                          line spacing after @a line_spacing argument has been applied.
     */
    [[nodiscard]] text_shaper(
        std::vector<attributed_grapheme> const &text,
        tt::vertical_alignment vertical_alignment,
        float line_spacing = 1.0f,
        float paragraph_spacing = 1.5f) noexcept;

    /** Get the natural rectangle of the text.
     *
     * This will load all the default glyphs for the text.
     * Then it will estimate the width and height based on the glyphs before glyph-morphing and kerning.
     * This may cause both over and under estimation of the width.
     *
     * @return The rectangle surrounding the text, excluding ascenders & descenders, as if
     *         each line is x-height. y = 0 is at the base-line based on the vertical alignment.
     *         The rectangle will be aligned to integers.
     */
    [[nodiscard]] aarectangle natural_bounding_rectangle() noexcept;

    /** Get the rectangle of the text after folding.
     *
     * This will fold based on the width of the glyphs before glyph-morphing and kerning.
     * This means if you pass the width of the natural rectangle from `bounding_rectangle()`
     * the text will not be folded.
     *
     * @return The rectangle surrounding the text, excluding ascenders & descenders, as if
     *         each line is x-height. y = 0 is at the base-line based on the vertical alignment.
     *         The rectangle will be aligned to integers.
     */
    [[nodiscard]] aarectangle bounding_rectangle() noexcept;

    /** Fold the text to width.
     */
    void fold(float width) noexcept;

private:
    struct char_type {
        using vector_type = std::vector<char_type>;
        using iterator = vector_type::iterator;

        /** The grapheme.
         */
        tt::grapheme grapheme;

        /** The style of how to display the grapheme.
         */
        tt::text_style style;

        /** The glyph representing one or more graphemes.
         * The glyph will change during shaping of the text:
         *  1. The initial glyph, used for determining the width of the grapheme
         *     and the folding algorithm.
         *  2. The glyph representing a bracket may be replaced with a mirrored bracket
         *     by the bidi-algorithm.
         *  3. The glyph may be replaced by the font using the glyph-morphing algorithms
         *     for better continuation of cursive text and merging of graphemes into
         *     a ligature.
         */
        tt::glyph_ids glyph;

        /** The glyph metrics of the currently glyph.
         *
         * The metrics are scaled by `style.size`.
         */
        tt::glyph_metrics metrics;

        /** The bounding rectangle for this character.
         *
         * The bounding rectangle is used to create selection-boxes,
         * cursor-position & mouse-position of the character.
         *
         * When multiple characters are converted to a ligature, the
         * bounding_rectangle of each of those characters occupies a
         * subsection of the ligature-glyph. In this case the left most
         * character will contain the ligature-glyph, and the rest of
         * the characters of the ligature will have empty glyphs.
         *
         * The left-bottom corner of the bounding_rectangle is used as the
         * offset for metrics.bounding_rectangle to display the glyph.
         */
        aarectangle bounding_rectangle;

        /** The glyph is the initial glyph.
         *
         * This flag is set to true after loading the initial glyph.
         * This flag is set to false when the glyph is replaced by the bidi-algorithm
         * or glyph-morphing.
         */
        bool glyph_is_initial = false;

        /** The width used for this grapheme when folding lines.
         *
         * This width is based on the initial glyph's advance after converting the grapheme
         * using the text-style into a glyph. This width excludes kerning and glyph-morphing.
         */
        float width = 0.0f;

        /** Initialize the glyph based on the grapheme.
         *
         * @note The glyph is only initialized when `glyph_is_initial == false`.
         * @post `glyph`, `metrics` and `width` are modified. `glyph_is_initial` is set to true.
         */
        void initialize_glyph() noexcept;

        /** Called by the bidi-algorithm to mirror glyphs.
         *
         * The glyph is replaced with a glyph from the same font using the given code-point.
         *
         * @pre `glyph.num_grapheme == 1`.
         * @post `glyph` and `metrics` are modified. `glyph_is_initial` is set to false.
         * @note The `width` remains based on the original glyph.
         */
        void replace_glyph(char32_t code_point) noexcept;
    };

    struct line_type {
        tt::font_metrics metrics;
        float y;
        float width;
    };

    struct paragraph_type {
        std::vector<char_type::iterator> chars_in_display_order;
        std::vector<line_type> lines;
    };

    enum class state_type { init, loaded, folded, complete };

    state_type _state;

    /** A list of character in logical order.
     *
     * @note Graphemes are not allowed to be typographical-ligatures.
     * @note The last grapheme must be a paragraph-separator.
     * @note line-feeds, carriage-returns & form-feeds must be replaced by paragraph-separators or line-separators.
     */
    std::vector<char_type> _chars_in_logical_order;
};

} // namespace tt::inline v1
