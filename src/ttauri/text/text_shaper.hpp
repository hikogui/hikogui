

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
 *  6. Position glyphs.
 */
class text_shaper {
public:
    text_shaper(std::vector<attributed_grapheme> const &attributed_graphemes, tt::alignment alignment) noexcept;

    /** Get the natural rectangle of the text.
     * This will load all the default glyphs for the text and do a quick and dirty layout
     * without folding.
     */
    aarectangle bounding_rectangle() noexcept;

    /** Fold the text to the given width.
     */
    void fold(float width) noexcept;

private:
    enum class state_type {
        init,
    };

    state_type _state;
};

}

