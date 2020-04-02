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

    /** The index into the text where the cursor is located.
     */
    ssize_t cursorIndex = 0;

    /** The index into the text where the start of the selection is located.
     * When no text is selected the cursorIndex and selectionIndex are equal.
     */
    ssize_t selectionIndex = 0;

    TextStyle currentStyle;

    /** Partial grapheme is inserted before cursorIndex.
     */
    bool hasPartialGrapheme = false;

public:
    EditableText(TextStyle style) :
        text(), _shapedText(), extent(0.0, 0.0), cursorIndex(0), selectionIndex(0), currentStyle(style)
    {
        // Make sure there is an end-paragraph marker in the text.
        // This allows the shapedText to figure out the style of the text of an empty paragraph.
        text.emplace_back(Grapheme('\n'), style, 0);
    }

    /** Update the shaped text after changed to text.
     */
    bool updateShapedText() noexcept {
        _shapedText = ShapedText(text, HorizontalAlignment::Left, extent.width());
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

    /** Return the text iterator at index.
     */
    decltype(auto) it(ssize_t index) noexcept {
        ttauri_assume(index >= 0);
        // Index should never be at text.cend();
        ttauri_assume(index < ssize(text));

        return text.begin() + index;
    }

    /** Return the text iterator at index.
    */
    decltype(auto) cit(ssize_t index) const noexcept {
        ttauri_assume(index >= 0);
        // Index should never be at text.cend();
        ttauri_assume(index < ssize(text));

        return text.cbegin() + index;
    }

    decltype(auto) it(ssize_t index) const noexcept {
        return cit(index);
    }

    /** Get carets at the cursor position.
     */
    rect leftToRightCaret() const noexcept {
        return _shapedText.leftToRightCaret(cursorIndex, false);
    }

    /** Get a set of rectangles for which text is selected.
     */
    std::vector<rect> selectionRectangles() const noexcept {
        auto r = std::vector<rect>{};
        if (selectionIndex < cursorIndex) {
            r = _shapedText.selectionRectangles(selectionIndex, cursorIndex);
        } else if (selectionIndex > cursorIndex) {
            r = _shapedText.selectionRectangles(cursorIndex, selectionIndex);
        }
        return r;
    }

    /** Delete a selection.
     * This function should be called when a selection is active while new text
     * is being inserted.
     *
     * @return true if text is modified.
     */
    bool deleteSelection() noexcept {
        if (selectionIndex < cursorIndex) {
            text.erase(cit(selectionIndex), cit(cursorIndex));
            cursorIndex = selectionIndex;
            return updateShapedText();
        } else if (selectionIndex > cursorIndex) {
            text.erase(cit(cursorIndex), cit(selectionIndex));
            selectionIndex = cursorIndex;
            return updateShapedText();
        } else {
            return false;
        }
    }

    /*! Find the nearest character at position and return it's index.
     */
    ssize_t characterIndexAtPosition(vec position) const noexcept;

    [[nodiscard]] bool setCursorAtCoordinate(vec coordinate) noexcept {
        if (let newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            selectionIndex = cursorIndex = *newCursorPosition;
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool selectWordAtCoordinate(vec coordinate) noexcept {
        if (let newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfWord(*newCursorPosition);
            return true;
        } else {
            return false;
        }
    }


    [[nodiscard]] bool dragCursorAtCoordinate(vec coordinate) noexcept {
        if (let newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            cursorIndex = *newCursorPosition;
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool dragWordAtCoordinate(vec coordinate) noexcept {
        if (let newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            let [a, b] = _shapedText.indicesOfWord(*newCursorPosition);

            if (selectionIndex <= cursorIndex) {
                if (a < selectionIndex) {
                    // Reverse selection
                    selectionIndex = cursorIndex;
                    cursorIndex = a;
                } else {
                    cursorIndex = b;
                }
            } else {
                if (b > selectionIndex) {
                    // Reverse selection
                    selectionIndex = cursorIndex;
                    cursorIndex = b;
                } else {
                    cursorIndex = a;
                }
            }
            return true;
        } else {
            return false;
        }
    }


    /*! Insert a temporary partial character.
     * This partial character is currently being constructed by the operating system.
     *
     * Since the insertion has not been completed any selected text should not yet be deleted.
     */
    bool insertPartialGrapheme(Grapheme character) noexcept {
        cancelPartialGrapheme();
        deleteSelection();

        text.emplace(cit(cursorIndex), character, currentStyle);
        selectionIndex = ++cursorIndex;
        hasPartialGrapheme = true;
        return updateShapedText();
    }

    bool cancelPartialGrapheme() noexcept {
        if (hasPartialGrapheme) {
            ttauri_assume(cursorIndex >= 1);

            selectionIndex = --cursorIndex;
            text.erase(cit(cursorIndex));
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
        cancelPartialGrapheme();
        deleteSelection();

        text.emplace(cit(cursorIndex), character, currentStyle);
        selectionIndex = ++cursorIndex;
        return updateShapedText();
    }

    bool handlePaste(std::string str) noexcept {
        cancelPartialGrapheme();
        deleteSelection();

        gstring gstr = to_gstring(str);

        auto str_attr = std::vector<AttributedGrapheme>{};
        str_attr.reserve(ssize(gstr));
        for (let &g: gstr) {
            str_attr.emplace_back(g, currentStyle);
        }

        text.insert(cit(cursorIndex), str_attr.cbegin(), str_attr.cend());
        selectionIndex = cursorIndex += ssize(str_attr);
        return updateShapedText();
    }

    std::string handleCopy() noexcept {
        auto r = std::string{};
        
        if (selectionIndex < cursorIndex) {
            r.reserve(cursorIndex - selectionIndex);
            for (auto i = cit(selectionIndex); i != cit(cursorIndex); ++i) {
                r += to_string(i->grapheme);
            }
        } else if (selectionIndex > cursorIndex) {
            r.reserve(selectionIndex - cursorIndex);
            for (auto i = cit(cursorIndex); i != cit(selectionIndex); ++i) {
                r += to_string(i->grapheme);
            }
        } 
        return r;
    }

    std::string handleCut() noexcept {
        auto r = handleCopy();
        cancelPartialGrapheme();
        deleteSelection();
        return r;
    }

    bool handleCommand(string_ltag command) noexcept {
        ttauri_assume(cursorIndex <= ssize(text));
        cancelPartialGrapheme();

        auto updated = false;

        if (command == "text.cursor.char.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                // XXX Change currentStyle based on the grapheme at the new cursor position.
                selectionIndex = cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.cursor.char.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.cursor.word.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.cursor.word.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.select.char.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.select.char.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.select.word.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.select.word.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                cursorIndex = *newCursorPosition;
                updated |= true;
            }
        } else if (command == "text.select.word"_ltag) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfWord(cursorIndex);
            updated |= true;
        } else if (command == "text.select.document"_ltag) {
            selectionIndex = 0;
            cursorIndex = size() - 1; // Upto end-of-paragraph marker.
            updated |= true;
            
        } else if (command == "text.delete.char.prev"_ltag) {
            if (cursorIndex != selectionIndex) {
                updated |= deleteSelection();

            } else if (cursorIndex >= 1) {
                selectionIndex = --cursorIndex;
                text.erase(cit(cursorIndex));
                updated |= updateShapedText();
            }
        } else if (command == "text.delete.char.next"_ltag) {
            if (cursorIndex != selectionIndex) {
                updated |= deleteSelection();

            } else if (cursorIndex < (ssize(text) - 1)) {
                // Don't delete the trailing paragraph separator.
                text.erase(cit(cursorIndex));
                updated |= updateShapedText();
            }
        }

        return updated;
    }
};


}

