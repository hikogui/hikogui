
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/grapheme.hpp"
#include <string>
#include <vector>

namespace TTauri {

class color_palette {
    int id;
    wsRGBA foregroundColor;
    wsRGBA backgroundColor;
    wsRGBA accentColor;
    wsRGBA shadowColor;
};

class text_style {
    int id;
    int fontId;
    int colorPaletteId;
    float fontSize;
    bool underlined;
    bool double_underlined;
    bool strike_through;
    bool wave;
};

class theme {
    Font fallbackFont;
    std::array<text_style,256> text_style;

};

/*! Editable text for GUI Widgets.
 *
 * The upper 11 bits of the first code point of a grapheme are used
 * as the style index for the style to use to render the grapheme.
 */
class text {
    std::vector<grapheme> graphemes;

    mutable size_t cursorPosition = 0;
    mutable ssize_t endSelection = -1;

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

    void undo() noexcept;

    void redo() noexcept;

    size_t undoSize() const noexcept;

    size_t redoSize() const noexcept;

    /*! Insert a temporary partial character.
     * This partial character is currently being constructed by the operating system.
     *
     * Since the insertion has not been completed a selected text should not yet be deleted.
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
};


}

