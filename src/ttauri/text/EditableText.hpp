// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "AttributedGrapheme.hpp"
#include "ShapedText.hpp"
#include "Font.hpp"
#include <string>
#include <vector>

namespace tt {

class EditableText {
    std::vector<AttributedGrapheme> text;
    ShapedText _shapedText;

    /** The maximum width when wrapping text.
     * For single line text editing, we should never wrap.
     */
    float width = 0.0f;

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
        text(), _shapedText(), currentStyle(style)
    {
    }

    /** Update the shaped text after changed to text.
     */
    void updateShapedText() noexcept {
        auto text_ = text;

        // Make sure there is an end-paragraph marker in the text.
        // This allows the shapedText to figure out the style of the text of an empty paragraph.
        if (ssize(text) == 0) {
            text_.emplace_back(Grapheme('\n'), currentStyle, 0);
        } else {
            text_.emplace_back(Grapheme('\n'), text_.back().style, 0);
        }

        _shapedText = ShapedText(text_, width, Alignment::TopLeft, false);
    }

    [[nodiscard]] ShapedText shapedText() const noexcept {
        return _shapedText;
    }

    void setWidth(float _width) noexcept {
        width = _width;
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
        tt_assume(index >= 0);
        // Index should never be at text.cend();
        tt_assume(index < ssize(text));

        return text.begin() + index;
    }

    /** Return the text iterator at index.
    */
    decltype(auto) cit(ssize_t index) const noexcept {
        tt_assume(index >= 0);
        // Index should never be beyond text.cend();
        tt_assume(index <= ssize(text));

        return text.cbegin() + index;
    }

    decltype(auto) it(ssize_t index) const noexcept {
        return cit(index);
    }

    /** Get carets at the cursor position.
    */
    aarect partialGraphemeCaret() const noexcept {
        if (hasPartialGrapheme) {
            tt_assume(cursorIndex != 0);
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
        if (ttlet newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            selectionIndex = cursorIndex = *newCursorPosition;
            tt_assume(selectionIndex >= 0);
            tt_assume(selectionIndex <= ssize(text));
        }
    }

    void selectWordAtCoordinate(vec coordinate) noexcept {
        if (ttlet newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfWord(*newCursorPosition);
            tt_assume(selectionIndex >= 0);
            tt_assume(selectionIndex <= ssize(text));
            tt_assume(cursorIndex >= 0);
            tt_assume(cursorIndex <= ssize(text));
        }
    }

    void selectParagraphAtCoordinate(vec coordinate) noexcept {
        if (ttlet newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfParagraph(*newCursorPosition);
            tt_assume(selectionIndex >= 0);
            tt_assume(selectionIndex <= ssize(text));
            tt_assume(cursorIndex >= 0);
            tt_assume(cursorIndex <= ssize(text));
        }
    }

    void dragCursorAtCoordinate(vec coordinate) noexcept {
        if (ttlet newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            cursorIndex = *newCursorPosition;
            tt_assume(cursorIndex >= 0);
            tt_assume(cursorIndex <= ssize(text));
        }
    }

    void dragWordAtCoordinate(vec coordinate) noexcept {
        if (ttlet newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            ttlet [a, b] = _shapedText.indicesOfWord(*newCursorPosition);

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

            tt_assume(selectionIndex >= 0);
            tt_assume(selectionIndex <= ssize(text));
            tt_assume(cursorIndex >= 0);
            tt_assume(cursorIndex <= ssize(text));
        }
    }

    void dragParagraphAtCoordinate(vec coordinate) noexcept {
        if (ttlet newCursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            ttlet [a, b] = _shapedText.indicesOfParagraph(*newCursorPosition);

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

            tt_assume(selectionIndex >= 0);
            tt_assume(selectionIndex <= ssize(text));
            tt_assume(cursorIndex >= 0);
            tt_assume(cursorIndex <= ssize(text));
        }
    }

    void cancelPartialGrapheme() noexcept {
        if (hasPartialGrapheme) {
            tt_assume(cursorIndex >= 1);

            selectionIndex = --cursorIndex;
            tt_assume(selectionIndex >= 0);
            tt_assume(selectionIndex <= ssize(text));
            tt_assume(cursorIndex >= 0);
            tt_assume(cursorIndex <= ssize(text));

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
        tt_assume(selectionIndex >= 0);
        tt_assume(selectionIndex <= ssize(text));
        tt_assume(cursorIndex >= 0);
        tt_assume(cursorIndex <= ssize(text));

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
        tt_assume(selectionIndex >= 0);
        tt_assume(selectionIndex <= ssize(text));
        tt_assume(cursorIndex >= 0);
        tt_assume(cursorIndex <= ssize(text));

        updateShapedText();
    }

    void handlePaste(std::string str) noexcept {
        cancelPartialGrapheme();
        deleteSelection();

        gstring gstr = to_gstring(str);

        auto str_attr = std::vector<AttributedGrapheme>{};
        str_attr.reserve(ssize(gstr));
        for (ttlet &g: gstr) {
            str_attr.emplace_back(g, currentStyle);
        }

        text.insert(cit(cursorIndex), str_attr.cbegin(), str_attr.cend());
        selectionIndex = cursorIndex += ssize(str_attr);
        tt_assume(selectionIndex >= 0);
        tt_assume(selectionIndex <= ssize(text));
        tt_assume(cursorIndex >= 0);
        tt_assume(cursorIndex <= ssize(text));

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
        tt_assume(cursorIndex <= ssize(text));
        cancelPartialGrapheme();

        if (command == "text.cursor.char.left"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                // XXX Change currentStyle based on the grapheme at the new cursor position.
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.char.right"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.word.left"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.word.right"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.word.right"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.cursor.line.end"_ltag) {
            selectionIndex = cursorIndex = size() - 1;
        } else if (command == "text.cursor.line.begin"_ltag) {
            selectionIndex = cursorIndex = 0;
        } else if (command == "text.select.char.left"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.select.char.right"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.select.word.left"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                cursorIndex = *newCursorPosition;
            }
        } else if (command == "text.select.word.right"_ltag) {
            if (ttlet newCursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
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

        tt_assume(selectionIndex >= 0);
        tt_assume(selectionIndex <= ssize(text));
        tt_assume(cursorIndex >= 0);
        tt_assume(cursorIndex <= ssize(text));
    }
};


}

