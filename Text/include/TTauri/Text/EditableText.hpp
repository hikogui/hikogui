// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Text/Font.hpp"
#include <string>
#include <vector>

namespace TTauri::Text {

class EditableText {
    std::vector<Grapheme> graphemes;

    mutable size_t cursorPosition = 0;
    mutable ssize_t endSelection = -1;
    mutable int styleAtCursor = 0;
    Grapheme partialCharacter = {};

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

    Grapheme &operator[](size_t i) noexcept {
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
    void insertPartialCharacter(Grapheme character) noexcept;

    /*! Cancel the temporary partial character.
     * Cancellation may happen when another widget or piece of text is selected by the user
     * during character construction.
     */
    void cancelPartialCharacter() noexcept;

    /*! insert character at the cursor position.
     * Selected text will be deleted.
     */
    void insertCharacter(Grapheme character) noexcept;

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

