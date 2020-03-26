// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/AttributedGrapheme.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Text/Font.hpp"
#include <string>
#include <vector>

namespace TTauri::Text {

class EditableText {
    std::vector<AttributedGrapheme> text;
    ShapedText _shapedText;

    vec extent;

    ssize_t cursorPosition = 0;
    ssize_t endSelection = -1;
    TextStyle currentStyle;

    /** Partial grapheme is inserted before cursorPosition.
     */
    bool hasPartialGrapheme = false;

public:
    EditableText(TextStyle style) :
        text(), _shapedText(), extent(0.0, 0.0), cursorPosition(0), endSelection(-1), currentStyle(style) {}


    /** Update the shaped text after changed to text.
     */
    bool updateShapedText() noexcept {
        _shapedText = ShapedText(text, extent, Alignment::TopLeft, true);
        return true;
    }

    [[nodiscard]] ShapedText shapedText() const noexcept {
        return _shapedText;
    }

    void setExtent(vec extent) noexcept {
        this->extent = extent;
        updateShapedText();
    }

    void setCurrentStyle(TextStyle style) noexcept {
        this->currentStyle = style;
    }

    size_t size() const noexcept {
        return text.size();
    }

    /** Get carets at the cursor position.
     */
    std::pair<vec,vec> carets() const noexcept {
        return _shapedText.carets(cursorPosition);
    }

    /*! Find the nearest character at position and return it's index.
     */
    size_t characterIndexAtPosition(vec position) const noexcept;

    /*! Find the nearest break between characters at position and return the index of the character after the break.
     */
    size_t breakIndexAtPosition(vec position) const noexcept;

    /*! Return the position of the character.
     */
    vec positionAtIndex(size_t index) const noexcept;

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
    //text copySelection() const noexcept;

    /*! Return and delete the selected text.
     */
    //text cutSelection() noexcept;

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
    bool insertPartialGrapheme(Grapheme character) noexcept {
        ttauri_assume(cursorPosition <= ssize(text));
        auto updated = cancelPartialGrapheme();

        text.emplace(text.cbegin() + cursorPosition++, character, currentStyle);
        hasPartialGrapheme = true;
        return updated | updateShapedText();
    }

    bool cancelPartialGrapheme() noexcept {
        ttauri_assume(cursorPosition <= ssize(text));

        if (hasPartialGrapheme) {
            ttauri_assume(cursorPosition >= 1);

            text.erase(text.cbegin() + --cursorPosition);
            hasPartialGrapheme = false;
            return updateShapedText();

        } else {
            return false;
        }
    }

    /*! insert character at the cursor position.
     * Selected text will be deleted.
     */
    bool insertGrapheme(Grapheme character) noexcept {
        ttauri_assume(cursorPosition <= ssize(text));
        cancelPartialGrapheme();

        text.emplace(text.cbegin() + cursorPosition++, character, currentStyle);
        return updateShapedText();
    }

    bool handleCommand(string_ltag command) noexcept {
        ttauri_assume(cursorPosition <= ssize(text));
        cancelPartialGrapheme();

        auto updated = false;

        if (command == "text.delete.char.prev"_ltag) {
            if (cursorPosition >= 1) {
                text.erase(text.cbegin() + --cursorPosition);
                updated |= updateShapedText();
            }
        }

        return updated;
    }
};


}

