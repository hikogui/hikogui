

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
     * @param alignment The alignment to use for the text.
     * @param paragraph_spacing A multiplier to scale the distance between lines compared to the
     *                          line spacing after @a line_spacing argument has been applied.
     * @param line_spacing A multiplier to scale the distance between lines compared to the
     *                     natural line spacing of the font: ascender + descender + line-gap.
     */
    [[nodiscard]] text_shaper(
        std::vector<attributed_grapheme> const &text,
        tt::alignment alignment,
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
    enum class state_type { init, loaded, folded, complete };

    state_type _state;
};

}

