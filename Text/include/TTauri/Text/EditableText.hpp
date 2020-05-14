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

    /** Insert-mode vs overwrite-mode.
     */
    bool insertMode = true;

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
    void updateShapedText() noexcept {
        _shapedText = ShapedText(text, Alignment::TopLeft, extent.width());
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

    /** Change the text style of all graphemes.
     */
    void setStyleOfAll(TextStyle style) noexcept {
        setCurrentStyle(style);
        for (auto &c: text) {
            c.style = style;
        }
        updateShapedText();
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
    aarect partialGraphemeCaret() const noexcept {
        if (hasPartialGrapheme) {
            ttauri_assume(cursorIndex != 0);
            return _shapedText.leftToRightCaret(cursorIndex - 1, false);
        } else {
            return {};
        }
    }

    /** Get carets at the cursor position.
     */
    aarect leftToRightCaret() const noexcept {
        return _shapedText.leftToRightCaret(cursorIndex, insertMode);
    }

    /** Get a set of rectangles for which text is selected.
     */
    std::vector<aarect> selectionRectangles() const noexcept {
        auto r = std::vector<aarect>{};
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
     */
    void deleteSelection() noexcept {
        if (selectionIndex < cursorIndex) {
            text.erase(cit(selectionIndex), cit(cursorIndex));
            cursorIndex = selectionIndex;
            updateShapedText();
        } else if (selectionIndex > cursorIndex) {
            text.erase(cit(cursorIndex), cit(selectionIndex));
            selectionIndex = cursorIndex;
            updateShapedText();
        }
    }

    /*! Find the nearest character at position and return it's index.
     */
    ssize_t characterIndexAtPosition(vec position) const noexcept;

    void setCursorAtCoordinate(vec coordinate) noexcept {
        if (let newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            selectionIndex = cursorIndex = *newCursorPosition;
        }
    }

    void selectWordAtCoordinate(vec coordinate) noexcept {
        if (let newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfWord(*newCursorPosition);
        }
    }


    void dragCursorAtCoordinate(vec coordinate) noexcept {
        if (let newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            cursorIndex = *newCursorPosition;
        }
    }

    void dragWordAtCoordinate(vec coordinate) noexcept {
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
        }
    }

    void cancelPartialGrapheme() noexcept {
        if (hasPartialGrapheme) {
            ttauri_assume(cursorIndex >= 1);

            selectionIndex = --cursorIndex;
            text.erase(cit(cursorIndex));
            hasPartialGrapheme = false;
            updateShapedText();
        }
    }

    /*! Insert a temporary partial character.
     * This partial character is currently being constructed by the operating system.
     *
     * Since the insertion has not been completed any selected text should not yet be deleted.
     */
    void insertPartialGrapheme(Grapheme character) noexcept {
        cancelPartialGrapheme();
        deleteSelection();

        text.emplace(cit(cursorIndex), character, currentStyle);
        selectionIndex = ++cursorIndex;
        hasPartialGrapheme = true;
        updateShapedText();
    }

    /*! insert character at the cursor position.
     * Selected text will be deleted.
     */
    void insertGrapheme(Grapheme character) noexcept {
        cancelPartialGrapheme();
        deleteSelection();

        if (!insertMode) {
            handleCommand("text.delete.char.next"_ltag);
        }
        text.emplace(cit(cursorIndex), character, currentStyle);
        selectionIndex = ++cursorIndex;
        updateShapedText();
    }

    void handlePaste(std::string str) noexcept {
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
        updateShapedText();
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

    void handleCommand(string_ltag command) noexcept {
        ttauri_assume(cursorIndex <= ssize(text));
        cancelPartialGrapheme();

        if (command == "text.cursor.char.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                // XXX Change currentStyle based on the grapheme at the new cursor position.
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.char.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.word.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.word.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.word.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.line.end"_ltag) {
            selectionIndex = cursorIndex = size() - 1;
        } else if (command == "text.cursor.line.begin"_ltag) {
            selectionIndex = cursorIndex = 0;
        } else if (command == "text.select.char.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.select.char.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.select.word.left"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.select.word.right"_ltag) {
            if (let newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.select.word"_ltag) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfWord(cursorIndex);
        } else if (command == "text.select.line.end"_ltag) {
            cursorIndex = size() - 1;
        } else if (command == "text.select.line.begin"_ltag) {
            cursorIndex = 0;
        } else if (command == "text.select.document"_ltag) {
            selectionIndex = 0;
            cursorIndex = size() - 1; // Upto end-of-paragraph marker.
        } else if (command == "text.mode.insert"_ltag) {
            insertMode = !insertMode;
        } else if (command == "text.delete.char.prev"_ltag) {
            if (cursorIndex != selectionIndex) {
                deleteSelection();

            } else if (cursorIndex >= 1) {
                selectionIndex = --cursorIndex;
                text.erase(cit(cursorIndex));
                updateShapedText();
            }
        } else if (command == "text.delete.char.next"_ltag) {
            if (cursorIndex != selectionIndex) {
                deleteSelection();

            } else if (cursorIndex < (ssize(text) - 1)) {
                // Don't delete the trailing paragraph separator.
                text.erase(cit(cursorIndex));
                updateShapedText();
            }
        }
    }
};


}

