// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/grapheme.hpp"
#include "TTauri/Foundation/Font.hpp"
#include "TTauri/Foundation/Theme.hpp"
#include <string>
#include <vector>

namespace TTauri {


class decorated_grapheme {
    /*! The code units representing the grapheme.
     * The grapheme is in Unicode-NFC after decomposing 'canonical' ligatures.
     * 'Canonical'-ligatures are those ligatures that have the same meaning,
     * in every language, when separated into individual characters,
     * and are only combined for purpose to improve legibility and style by the font.
     */
    grapheme graphemeCodePoints;

    /*! Text style for this grapheme.
     * 18'0000 - 1f'ffff Code points to represent a textStyle, 19 bits in total.
     *
     * - 18:16 - 8 different decorations (Normal, Underlines, Dashed Underline, DoubleUnderline, WavyUnderline, Strikethrough)
     * - 15:13 - 8 different decoration colors.
     * - 12:10 - 8 font-shape (regular, italic, bold, boldItalic, light, lightItalic)
     * -  9:8  - 4 font-families (Serif, Sans, Condensed, Monospace)
     * -  7:4  - 16 font sizes (8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 28, 32, 50, 64, 100)
     * -  3:0  - 16 different text colors.
     */
    TextStyle textStyle;

    /*! Font where the grapheme was found.
     * The font is selected based on an algorithm which priorities in the following order:
     *  1. Try ligature combinations with the next graphemes.
     *  2. Try NFC form, which will use less pre-composed glyphs in the font.
     *  3. Try NFD form, which will require more glyphs to combine the character.
     *  4. Try the next fall back font in the list and goto 1.
     *  5. Use the Unknown character glyph from the first font.
     */
    Font *font;

    /*! The set of glyphs which match the grapheme.
     * Potentially the glyphs in this list may represent multiple graphemes when
     * a ligature was combined.
     */
    std::vector<int> glyphIndices;

    /*! Number of graphemes that the glyphIndices represent.
     * Potentially the glyphs in this list may represent multiple graphemes when
     * a ligature was combined.
     */
    size_t nrGraphemesInGlyphs;

    /*! Metrics loaded for each glyph in font:glyphIndices.
     */
    std::vector<GlyphsMetrics> glyphsMetrics;

    /*! Merged metrics from glyphsMetrics.
     */
    GlyphsMetric metrics;
public:

};

/*! Editable text for GUI Widgets.
 *
 * When converting between text and u32string and back certain code points
 * have special meaning:
 *
 *   0x00'0000 - 0x10'ffff Unicode code points plane 0 to plane 16.
 *   0x11'0000             Push formatting
 *   0x11'0001             Pop formatting
 *   0x11'01xx             Select font color.
 *   0x11'02xx             Select decoration color.
 *   0x11'03xx             Select decoration.
 *                         bit  3:0 Line style: (solid line, dotted line, dashed line, double line, wavy line)
 *                         bit  7:4 Line location: (no line, line above, line under, line through)
 *   0x11'1xxx             Select font size.
 *   0x18'0000 - 0x1f'ffff Select font from font registry 19 bits.
 *                         bit    0 Italic
 *                         bit  3:1 Font weight:
 *                                  0=200 Extra-Light,1=300 Light,2=400 Regular,3=500 Medium,
 *                                  4=600 Semi-Bold,5=700 Bold,6=800 Extra-Bold,7=900 Black
 *                         bit  5:4 Font Category: 0=Normal, 1=Serif, 2=Mono-space, 3=Condensed
 *                         bit 18:6 Font family.
 *
 */
class text {
    std::vector<decorated_grapheme> graphemes;

    mutable size_t cursorPosition = 0;
    mutable ssize_t endSelection = -1;
    mutable int styleAtCursor = 0;
    grapheme partialCharacter = {};

public:
    auto begin() noexcept {
        return graphemes.begin();
    }

    auto end() noexcept {
        return graphemes.end();
    }

    size_t size() const noexcept {
        return graphemes.size();
    }

    grapheme &operator[](size_t i) noexcept {
        return graphemes[i];
    }

    /*! Find the nearest character at position and return it's index.
     */
    size_t characterIndexAtPosition(glm::vec2 position) const noexcept;

    /*! Find the nearest break between characters at position and return the index of the character after the break.
     */
    size_t breakIndexAtPosition(glm::vec2 position) const noexcept;

    /*! Return the position of the character.
     */
    glm::vec2 positionAtIndex(size_t index) const noexcept;

    /*! Set cursor at the break before the character at the index.
     */
    void setCursorPosition(size_t index) const noexcept;

    /*! Select a block of text.
     * The cursor position will be set at the break before the
     * first selected character.
     *
     * \param begin Index of the first selected character.
     * \param end Index of the last selected character.
     */
    void setSelection(size_t begin, size_t end) const noexcept;

    /*! Cancel selection of text.
     * This will happen when something else gets selected.
     */
    void cancelSelection() const noexcept;

    /*! Delete the selected text.
     */
    void deleteSelection() noexcept;

    /*! Return the selected text.
     */
    text copySelection() const noexcept;

    /*! Return and delete the selected text.
     */
    text cutSelection() noexcept;

    /*! Set the current style.
     * If text is selected the style of the selected text changes.
     * Otherwise the style at the cursor changes.
     */
    void setStyle(int styleIndex) noexcept;

    /*! Get the current style.
     * This is the style at the cursor or the style of the first selected character.
     * The style at the cursor is determined when the cursor position changes or
     * by a call to setStyle().
     *
     * If the cursor moved from left->right or up->down the cursor style is taken from
     * the character directly before the new cursor position.
     *
     * If the cursor moved from right->left or down->up the cursor style is taken from
     * the character directly after the new cursor position.
     */
    int getStyle(void) const noexcept;

    /*! Undo a text operation.
     */
    void undo() noexcept;

    /*! Redo after an undo().
    */
    void redo() noexcept;

    /*! Number of operations that can be undone.
     */
    size_t undoSize() const noexcept;

    /*! Number of undo() operations that can be redone.
     */
    size_t redoSize() const noexcept;

    /*! Insert a temporary partial character.
     * This partial character is currently being constructed by the operating system.
     *
     * Since the insertion has not been completed any selected text should not yet be deleted.
     */
    void insertPartialCharacter(grapheme character) noexcept;

    /*! Cancel the temporary partial character.
     * Cancellation may happen when another widget or piece of text is selected by the user
     * during character construction.
     */
    void cancelPartialCharacter() noexept;

    /*! insert character at the cursor position.
     * Selected text will be deleted.
     */
    void insertCharacter(grapheme character) noexcept;

    /*! insert text at the cursor position.
     * Selected text will be deleted.
     */
    void pasteText(text text) noexcept;

private:
    /*! Calculate metrics and position for each grapheme.
     */
    void shapeText() noexcept;
};


}

