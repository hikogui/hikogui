// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "attributed_grapheme.hpp"
#include "shaped_text.hpp"
#include "font.hpp"
#include <string>
#include <vector>

namespace tt {

class editable_text {
    std::vector<attributed_grapheme> text;
    shaped_text _shapedText;

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

    text_style currentStyle;

    /** Partial grapheme is inserted before cursorIndex.
     */
    bool hasPartialgrapheme = false;

public:
    editable_text(text_style style) :
        text(), _shapedText(), currentStyle(style)
    {
    }

    [[nodiscard]] operator std::string() const noexcept
    {
        auto r = std::string{};
        
        for (ttlet &c : text) {
            r += to_string(c.grapheme.NFC());
        }

        return r;
    }

    editable_text &operator=(std::string_view str) noexcept
    {
        cancelPartialgrapheme();

        gstring gstr = to_gstring(str);

        text.clear();
        text.reserve(std::ssize(gstr));
        for (ttlet &g : gstr) {
            text.emplace_back(g, currentStyle);
        }

        selectionIndex = cursorIndex = 0;
        tt_axiom(selectionIndex >= 0);
        tt_axiom(selectionIndex <= std::ssize(text));
        tt_axiom(cursorIndex >= 0);
        tt_axiom(cursorIndex <= std::ssize(text));

        updateshaped_text();
        return *this;
    }

    /** Update the shaped text after changed to text.
     */
    void updateshaped_text() noexcept {
        auto text_ = text;

        // Make sure there is an end-paragraph marker in the text.
        // This allows the shapedText to figure out the style of the text of an empty paragraph.
        if (std::ssize(text) == 0) {
            text_.emplace_back(grapheme::PS(), currentStyle, 0);
        } else {
            text_.emplace_back(grapheme::PS(), text_.back().style, 0);
        }

        _shapedText = shaped_text(text_, width, alignment::top_left, false);
    }

    [[nodiscard]] shaped_text shapedText() const noexcept {
        return _shapedText;
    }

    void setWidth(float _width) noexcept {
        width = _width;
        updateshaped_text();
    }

    void setCurrentStyle(text_style style) noexcept {
        this->currentStyle = style;
    }

    /** Change the text style of all graphemes.
     */
    void setStyleOfAll(text_style style) noexcept {
        setCurrentStyle(style);
        for (auto &c: text) {
            c.style = style;
        }
        updateshaped_text();
    }

    size_t size() const noexcept {
        return text.size();
    }

    /** Return the text iterator at index.
     */
    decltype(auto) it(ssize_t index) noexcept {
        tt_axiom(index >= 0);
        // Index should never be at text.cend();
        tt_axiom(index < std::ssize(text));

        return text.begin() + index;
    }

    /** Return the text iterator at index.
    */
    decltype(auto) cit(ssize_t index) const noexcept {
        tt_axiom(index >= 0);
        // Index should never be beyond text.cend();
        tt_axiom(index <= std::ssize(text));

        return text.cbegin() + index;
    }

    decltype(auto) it(ssize_t index) const noexcept {
        return cit(index);
    }

    /** Get carets at the cursor position.
    */
    aarectangle partialgraphemeCaret() const noexcept {
        if (hasPartialgrapheme) {
            tt_axiom(cursorIndex != 0);
            return _shapedText.leftToRightCaret(cursorIndex - 1, false);
        } else {
            return {};
        }
    }

    /** Get carets at the cursor position.
     */
    aarectangle leftToRightCaret() const noexcept {
        return _shapedText.leftToRightCaret(cursorIndex, insertMode);
    }

    /** Get a set of rectangles for which text is selected.
     */
    std::vector<aarectangle> selectionRectangles() const noexcept {
        auto r = std::vector<aarectangle>{};
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
            updateshaped_text();
        } else if (selectionIndex > cursorIndex) {
            text.erase(cit(cursorIndex), cit(selectionIndex));
            selectionIndex = cursorIndex;
            updateshaped_text();
        }
    }

    /*! Find the nearest character at position and return it's index.
     */
    ssize_t characterIndexAtPosition(point2 position) const noexcept;

    void setmouse_cursorAtCoordinate(point2 coordinate) noexcept
    {
        if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            selectionIndex = cursorIndex = *newmouse_cursorPosition;
            tt_axiom(selectionIndex >= 0);
            tt_axiom(selectionIndex <= std::ssize(text));
        }
    }

    void selectWordAtCoordinate(point2 coordinate) noexcept
    {
        if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfWord(*newmouse_cursorPosition);
            tt_axiom(selectionIndex >= 0);
            tt_axiom(selectionIndex <= std::ssize(text));
            tt_axiom(cursorIndex >= 0);
            tt_axiom(cursorIndex <= std::ssize(text));
        }
    }

    void selectParagraphAtCoordinate(point2 coordinate) noexcept
    {
        if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfParagraph(*newmouse_cursorPosition);
            tt_axiom(selectionIndex >= 0);
            tt_axiom(selectionIndex <= std::ssize(text));
            tt_axiom(cursorIndex >= 0);
            tt_axiom(cursorIndex <= std::ssize(text));
        }
    }

    void dragmouse_cursorAtCoordinate(point2 coordinate) noexcept
    {
        if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            cursorIndex = *newmouse_cursorPosition;
            tt_axiom(cursorIndex >= 0);
            tt_axiom(cursorIndex <= std::ssize(text));
        }
    }

    void dragWordAtCoordinate(point2 coordinate) noexcept
    {
        if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            ttlet [a, b] = _shapedText.indicesOfWord(*newmouse_cursorPosition);

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

            tt_axiom(selectionIndex >= 0);
            tt_axiom(selectionIndex <= std::ssize(text));
            tt_axiom(cursorIndex >= 0);
            tt_axiom(cursorIndex <= std::ssize(text));
        }
    }

    void dragParagraphAtCoordinate(point2 coordinate) noexcept
    {
        if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharAtCoordinate(coordinate)) {
            ttlet [a, b] = _shapedText.indicesOfParagraph(*newmouse_cursorPosition);

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

            tt_axiom(selectionIndex >= 0);
            tt_axiom(selectionIndex <= std::ssize(text));
            tt_axiom(cursorIndex >= 0);
            tt_axiom(cursorIndex <= std::ssize(text));
        }
    }

    void cancelPartialgrapheme() noexcept {
        if (hasPartialgrapheme) {
            tt_axiom(cursorIndex >= 1);

            selectionIndex = --cursorIndex;
            tt_axiom(selectionIndex >= 0);
            tt_axiom(selectionIndex <= std::ssize(text));
            tt_axiom(cursorIndex >= 0);
            tt_axiom(cursorIndex <= std::ssize(text));

            text.erase(cit(cursorIndex));
            hasPartialgrapheme = false;

            updateshaped_text();
        }
    }

    /*! Insert a temporary partial character.
     * This partial character is currently being constructed by the operating system.
     *
     * Since the insertion has not been completed any selected text should not yet be deleted.
     */
    void insertPartialgrapheme(grapheme character) noexcept {
        cancelPartialgrapheme();
        deleteSelection();

        text.emplace(cit(cursorIndex), character, currentStyle);
        selectionIndex = ++cursorIndex;
        tt_axiom(selectionIndex >= 0);
        tt_axiom(selectionIndex <= std::ssize(text));
        tt_axiom(cursorIndex >= 0);
        tt_axiom(cursorIndex <= std::ssize(text));

        hasPartialgrapheme = true;
        updateshaped_text();
    }

    /*! insert character at the cursor position.
     * Selected text will be deleted.
     */
    void insertgrapheme(grapheme character) noexcept {
        cancelPartialgrapheme();
        deleteSelection();

        if (!insertMode) {
            handle_event(command::text_delete_char_next);
        }
        text.emplace(cit(cursorIndex), character, currentStyle);
        selectionIndex = ++cursorIndex;
        tt_axiom(selectionIndex >= 0);
        tt_axiom(selectionIndex <= std::ssize(text));
        tt_axiom(cursorIndex >= 0);
        tt_axiom(cursorIndex <= std::ssize(text));

        updateshaped_text();
    }

    void handlePaste(std::string str) noexcept {
        cancelPartialgrapheme();
        deleteSelection();

        gstring gstr = to_gstring(str);

        auto str_attr = std::vector<attributed_grapheme>{};
        str_attr.reserve(std::ssize(gstr));
        for (ttlet &g: gstr) {
            str_attr.emplace_back(g, currentStyle);
        }

        text.insert(cit(cursorIndex), str_attr.cbegin(), str_attr.cend());
        selectionIndex = cursorIndex += std::ssize(str_attr);
        tt_axiom(selectionIndex >= 0);
        tt_axiom(selectionIndex <= std::ssize(text));
        tt_axiom(cursorIndex >= 0);
        tt_axiom(cursorIndex <= std::ssize(text));

        updateshaped_text();
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
        cancelPartialgrapheme();
        deleteSelection();
        return r;
    }

    bool handle_event(command command) noexcept {
        auto handled = false;

        tt_axiom(cursorIndex <= std::ssize(text));
        cancelPartialgrapheme();

        switch (command) {
        case command::text_cursor_char_left:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                // XXX Change currentStyle based on the grapheme at the new cursor position.
                selectionIndex = cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_cursor_char_right:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_cursor_word_left:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                selectionIndex = cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_cursor_word_right:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                selectionIndex = cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_cursor_line_end:
            handled = true;
            selectionIndex = cursorIndex = size() - 1;
            break;

        case command::text_cursor_line_begin:
            handled = true;
            selectionIndex = cursorIndex = 0;
            break;

        case command::text_select_char_left:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharOnTheLeft(cursorIndex)) {
                cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_select_char_right:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfCharOnTheRight(cursorIndex)) {
                cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_select_word_left:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfWordOnTheLeft(cursorIndex)) {
                cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_select_word_right:
            handled = true;
            if (ttlet newmouse_cursorPosition = _shapedText.indexOfWordOnTheRight(cursorIndex)) {
                cursorIndex = *newmouse_cursorPosition;
            }
            break;

        case command::text_select_word:
            handled = true;
            std::tie(selectionIndex, cursorIndex) = _shapedText.indicesOfWord(cursorIndex);
            break;

        case command::text_select_line_end:
            handled = true;
            cursorIndex = size() - 1;
            break;

        case command::text_select_line_begin:
            handled = true;
            cursorIndex = 0;
            break;

        case command::text_select_document:
            handled = true;
            selectionIndex = 0;
            cursorIndex = size() - 1; // Upto end-of-paragraph marker.
            break;

        case command::text_mode_insert:
            handled = true;
            insertMode = !insertMode;
            break;

        case command::text_delete_char_prev:
            handled = true;
            if (cursorIndex != selectionIndex) {
                deleteSelection();

            } else if (cursorIndex >= 1) {
                selectionIndex = --cursorIndex;
                text.erase(cit(cursorIndex));
                updateshaped_text();
            }
            break;

        case command::text_delete_char_next:
            handled = true;
            if (cursorIndex != selectionIndex) {
                deleteSelection();

            } else if (cursorIndex < (std::ssize(text) - 1)) {
                // Don't delete the trailing paragraph separator.
                text.erase(cit(cursorIndex));
                updateshaped_text();
            }
        default:;
        }

        tt_axiom(selectionIndex >= 0);
        tt_axiom(selectionIndex <= std::ssize(text));
        tt_axiom(cursorIndex >= 0);
        tt_axiom(cursorIndex <= std::ssize(text));
        return handled;
    }
};


}

