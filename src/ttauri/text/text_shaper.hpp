// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../alignment.hpp"
#include "../coroutine.hpp"
#include "glyph_metrics.hpp"
#include "font_metrics.hpp"
#include "grapheme.hpp"
#include "gstring.hpp"
#include "text_style.hpp"
#include "glyph_ids.hpp"
#include "unicode_description.hpp"
#include "font.hpp"
#include <vector>

#pragma once

namespace tt::inline v1 {
class font_book;

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
 *  6. Position glyphs including kerning and justification.
 *
 */
class text_shaper {
public:
    constexpr text_shaper() noexcept = default;
    constexpr text_shaper(text_shaper const &) noexcept = default;
    constexpr text_shaper(text_shaper &&) noexcept = default;
    constexpr text_shaper &operator=(text_shaper const &) noexcept = default;
    constexpr text_shaper &operator=(text_shaper &&) noexcept = default;

    /** Construct a text_shaper with a text and alignment.
     *
     * The constructor will load all the default glyphs for the text.
     *
     * Horizontal alignment is done for each line independent of the writing direction.
     * This allows labels to remain aligned in the same direction on the user-interface
     * even when the labels have translations in different languages.
     *
     * Label widgets should flip the alignment passed to the text shaper when the
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
    [[nodiscard]] text_shaper(tt::font_book &font_book, gstring const &text, text_style const &style) noexcept;

    [[nodiscard]] text_shaper(font_book &font_book, std::string_view text, text_style const &style) noexcept;

    /** Layout the lines of the text.
     *
     * It will estimate the width and height based on the glyphs before glyph-morphing and kerning
     * and fold the lines using the unicode line breaking algorithm to the @a max_line_width.
     *
     * The @a alignment parameter is used to align the lines vertically:
     *  - top: y=0 is the base-line of the top line, with following lines below it.
     *  - bottom: y=0 is the base-line of the bottom line, with previous lines above it.
     *  - middle, odd number of lines: y=0 is the base-line of the middle line.
     *  - middle, even number of lines: y=0 is half-way between the base-line of the two lines in the middle.
     *
     * @post The lines have been laid out.
     * @param maximum_line_width The maximum line width allowed, this may be infinite to determine
     *        the natural text size without folding.
     * @param alignment The vertical alignment of text.
     * @param line_spacing The scaling of the spacing between lines.
     * @param paragraph_spacing The scaling of the spacing between paragraphs.
     * @return The rectangle surrounding the text, excluding ascenders & descenders, as if
     *         each line is x-height. y = 0 is the base-line of the text.
     */
    [[nodiscard]] aarectangle layout_lines(
        float maximum_line_width,
        vertical_alignment alignment,
        float line_spacing = 1.0f,
        float paragraph_spacing = 1.5f) noexcept;

    /** Position all the glyphs of the text.
     *
     * @pre `layout_lines()` must be called to layout the lines.
     * @post The glyphs have been positioned.
     * @param rectangle The rectangle to position the glyphs in.
     * @param base_line The position of the recommended base-line. If the text does not fit in the rectangle using the given
     *        base-line, then the base-line will be ignored.
     * @param alignment The horizontal alignment of the text
     * @param writing_direction The default writing direction, either `L` (left-to-right), `R` (right-to-left) or `unknown`
     *        when the writing direction is unknown.
     * @param sub_pixel_size The size of a sub-pixel in device-independent-pixels. This value is use to round coordinates of each
     *        glyph.
     */
    void position_glyphs(
        aarectangle rectangle,
        float base_line,
        text_alignment alignment,
        unicode_bidi_class writing_direction,
        extent2 sub_pixel_size) noexcept;

    /** find the nearest character.
     *
     * @param point The point near
     * @return The index to the character that is nearest to the point.
     */
    [[nodiscard]] ssize_t get_nearest(point2 point) const noexcept;

    /** Get the character to the left.
     *
     * @param index The index to a character.
     * @return The index to the character on the left,
     *         or the most right character in the line above,
     *         or -1 if before begin of text.
     */
    [[nodiscard]] ssize_t left_of(ssize_t index) const noexcept;

    /** Get the character to the right.
     *
     * @param index The index to a character.
     * @return The index to the character on the right,
     *         or the most left character in the line below,
     *         or one beyond the end of text.
     */
    [[nodiscard]] ssize_t right_of(ssize_t index) const noexcept;

    /** Get the character above.
     *
     * @param index The index to a character.
     * @return The index to the character above,
     *         or -1 if before begin of text.
     */
    [[nodiscard]] ssize_t above(ssize_t index) const noexcept;

    /** Get the character to the right.
     *
     * @param index The index to a character.
     * @return The index to the character below,
     *         or one beyond the end of text.
     */
    [[nodiscard]] ssize_t below(ssize_t index) const noexcept;

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
         * The metrics are scaled by `scale`.
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

        /** The unicode description of the grapheme.
         */
        unicode_description const *description;

        /** The scale of the glyph for displaying on the screen.
         */
        float scale = 1.0f;

        /** The width used for this grapheme when folding lines.
         *
         * This width is based on the initial glyph's advance after converting the grapheme
         * using the text-style into a glyph. This width excludes kerning and glyph-morphing.
         */
        float width = 0.0f;

        /** The glyph is the initial glyph.
         *
         * This flag is set to true after loading the initial glyph.
         * This flag is set to false when the glyph is replaced by the bidi-algorithm
         * or glyph-morphing.
         */
        bool glyph_is_initial = false;

        [[nodiscard]] char_type(tt::grapheme const &grapheme, text_style const &style) noexcept;

        /** Initialize the glyph based on the grapheme.
         *
         * @note The glyph is only initialized when `glyph_is_initial == false`.
         * @post `glyph`, `metrics` and `width` are modified. `glyph_is_initial` is set to true.
         */
        void initialize_glyph(tt::font_book &font_book, tt::font const &font) noexcept;

        /** Initialize the glyph based on the grapheme.
         *
         * @note The glyph is only initialized when `glyph_is_initial == false`.
         * @post `glyph`, `metrics` and `width` are modified. `glyph_is_initial` is set to true.
         */
        void initialize_glyph(tt::font_book &font_book) noexcept;

        /** Called by the bidi-algorithm to mirror glyphs.
         *
         * The glyph is replaced with a glyph from the same font using the given code-point.
         *
         * @pre `glyph.num_grapheme == 1`.
         * @post `glyph` and `metrics` are modified. `glyph_is_initial` is set to false.
         * @note The `width` remains based on the original glyph.
         */
        void replace_glyph(char32_t code_point) noexcept;

        /** Get the scaled font metrics for this character.
         */
        [[nodiscard]] tt::font_metrics font_metrics() const noexcept
        {
            return scale * glyph.font().metrics;
        }

    private:
        /** Load metrics based on the loaded glyph.
         */
        void set_glyph(tt::glyph_ids &&new_glyph) noexcept;
    };
    using char_vector = std::vector<char_type>;
    using char_iterator = char_vector::iterator;
    using char_const_iterator = char_vector::const_iterator;

    struct line_type {
        char_const_iterator first;
        char_const_iterator last;

        /** Indices to the characters in the text.
         *
         * The indices are in display-order.
         */
        std::vector<char_iterator> columns;

        /** The maximum metrics of the font of each glyph on this line.
         */
        tt::font_metrics metrics;

        /** Position of the base-line of this line.
         */
        float y;

        /** The width of this line, excluding trailing white space, glyph morphing and kerning.
         */
        float width;

        /** True if this line ends a paragraph.
         */
        bool end_of_paragraph;

        /** The writing direction of the paragraph.
         *
         * This value will be set the same on each line of a paragraph.
         */
        unicode_bidi_class paragraph_direction;

        /** The alignment of the paragraph.
         *
         * The paragraph_alignment is resolved after determining the
         * writing direction of the paragraph. specifically `flush`
         * is resolved to `flush_left` or `flush_right`.
         * 
         * This value will be set the same on each line of a paragraph.
         */
        text_alignment paragraph_alignment;

        /** Construct a line.
         *
         * @param begin The first character of the text.
         * @param first The first character of the line.
         * @param last One beyond the last character of the line.
         */
        line_type(text_shaper::char_const_iterator begin, char_const_iterator first, char_const_iterator last) noexcept;

    private:
        /** Get the length of a line.
         *
         * @param first The first character of the line.
         * @param last One beyond the last character of the line.
         * @return The width of the line, excluding trailing white-space.
         */
        [[nodiscard]] float calculate_width() noexcept;
    };
    using line_vector = std::vector<line_type>;
    using line_iterator = line_vector::iterator;
    using line_const_iterator = line_vector::const_iterator;

    enum class state_type { init, loaded, folded, complete };

    state_type _state;

    font_book *_font_book = nullptr;

    /** A list of character in logical order.
     *
     * @note Graphemes are not allowed to be typographical-ligatures.
     * @note The last grapheme must be a paragraph-separator.
     * @note line-feeds, carriage-returns & form-feeds must be replaced by paragraph-separators or line-separators.
     */
    char_vector _text;

    vertical_alignment _vertical_alignment = vertical_alignment::middle;

    text_alignment _text_alignment = text_alignment::centered;

    float _line_spacing = 1.0f;

    float _paragraph_spacing = 1.0f;

    std::vector<line_type> _lines;

    /** Fold lines.
     *
     * @return A vector of number-of-characters for each line. A line is never zero characters long.
     */
    [[nodiscard]] std::vector<size_t> fold_lines(float maximum_line_width) const noexcept;

    [[nodiscard]] std::vector<line_type> layout_create_lines(float maximum_line_width) const noexcept;

    void layout_lines_vertical_spacing(float paragraph_spacing, float line_spacing) noexcept;
    [[nodiscard]] float layout_lines_vertical_adjustment(vertical_alignment alignment) const noexcept;
    void reorder_glyphs(unicode_bidi_class writing_direction) noexcept;
    void resolve_text_alignment(text_alignment alignment) noexcept;

    /** Get column and line of a character.
     */
    //[[nodiscard]] std::pair<ssize_t, ssize_t> text_shaper::get_column_line(ssize_t index) const noexcept;
};

} // namespace tt::inline v1
