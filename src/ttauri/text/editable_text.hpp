// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "attributed_grapheme.hpp"
#include "shaped_text.hpp"
#include "font.hpp"
#include "../ranges.hpp"
#include "../gap_buffer.hpp"
#include <string>
#include <vector>

namespace tt::inline v1 {

class editable_text {
public:
    editable_text(tt::font_book const &font_book, text_style style) :
        _font_book(font_book), _text(), _shaped_text(), _current_style(style)
    {
    }

    [[nodiscard]] operator std::string() const noexcept
    {
        auto r = std::string{};

        for (ttlet &c : _text) {
            r += to_string(c.grapheme.NFC());
        }

        return r;
    }

    editable_text &operator=(std::string_view str) noexcept
    {
        tt_axiom(is_valid());
        cancel_partial_grapheme();

        gstring gstr = to_gstring(str);

        _text.clear();
        _text.reserve(gstr.size());
        for (ttlet &g : gstr) {
            _text.emplace_back(g, _current_style);
        }

        _selection_index = _cursor_index = 0;

        update_shaped_text();
        tt_axiom(is_valid());
        return *this;
    }

    /** Update the shaped _text after changed to _text.
     */
    void update_shaped_text() noexcept
    {
        auto text_ = make_vector(_text);

        // Make sure there is an end-paragraph marker in the _text.
        // This allows the shaped_text to figure out the style of the _text of an empty paragraph.
        if (ssize(_text) == 0) {
            text_.emplace_back(grapheme::PS(), _current_style, 0);
        } else {
            text_.emplace_back(grapheme::PS(), text_.back().style, 0);
        }

        _shaped_text = tt::shaped_text{
            _font_book, text_, _width, tt::alignment{horizontal_alignment::left, vertical_alignment::top}, false};
    }

    [[nodiscard]] shaped_text shaped_text() const noexcept
    {
        return _shaped_text;
    }

    void set_width(float width) noexcept
    {
        _width = width;
        update_shaped_text();
    }

    void set_current_style(text_style style) noexcept
    {
        this->_current_style = style;
    }

    /** Change the text style of all graphemes.
     */
    void set_style_of_all(text_style style) noexcept
    {
        set_current_style(style);
        for (auto &c : _text) {
            c.style = style;
        }
        update_shaped_text();
    }

    std::size_t size() const noexcept
    {
        return _text.size();
    }

    /** Return the _text iterator at index.
     */
    decltype(auto) it(ssize_t index) noexcept
    {
        tt_axiom(index >= 0);
        // Index should never be at _text.cend();
        tt_axiom(index < ssize(_text));

        return _text.begin() + index;
    }

    /** Return the _text iterator at index.
     */
    decltype(auto) cit(ssize_t index) const noexcept
    {
        tt_axiom(index >= 0);
        // Index should never be beyond _text.cend();
        tt_axiom(index <= ssize(_text));

        return _text.cbegin() + index;
    }

    decltype(auto) it(ssize_t index) const noexcept
    {
        return cit(index);
    }

    /** Get carets at the cursor position.
     */
    aarectangle partial_grapheme_caret() const noexcept
    {
        tt_axiom(is_valid());

        if (_has_partial_grapheme) {
            tt_axiom(_cursor_index != 0);
            return _shaped_text.left_to_right_caret(_cursor_index - 1, false);
        } else {
            return {};
        }
    }

    /** Get carets at the cursor position.
     */
    aarectangle left_to_right_caret() const noexcept
    {
        tt_axiom(is_valid());
        return _shaped_text.left_to_right_caret(_cursor_index, _insert_mode);
    }

    /** Get carets at the cursor position.
     */
    aarectangle right_to_left_caret() const noexcept
    {
        tt_axiom(is_valid());
        return _shaped_text.right_to_left_caret(_cursor_index, _insert_mode);
    }

    /** Get a set of rectangles for which _text is selected.
     */
    std::vector<aarectangle> selection_rectangles() const noexcept
    {
        tt_axiom(is_valid());
        auto r = std::vector<aarectangle>{};
        if (_selection_index < _cursor_index) {
            r = _shaped_text.selection_rectangles(_selection_index, _cursor_index);
        } else if (_selection_index > _cursor_index) {
            r = _shaped_text.selection_rectangles(_cursor_index, _selection_index);
        }
        tt_axiom(is_valid());
        return r;
    }

    /** Delete a selection.
     * This function should be called when a selection is active while new _text
     * is being inserted.
     */
    void delete_selection() noexcept
    {
        tt_axiom(is_valid());

        if (_selection_index < _cursor_index) {
            _text.erase(cit(_selection_index), cit(_cursor_index));
            _cursor_index = _selection_index;
            update_shaped_text();
        } else if (_selection_index > _cursor_index) {
            _text.erase(cit(_cursor_index), cit(_selection_index));
            _selection_index = _cursor_index;
            update_shaped_text();
        }
        tt_axiom(is_valid());
    }

    /*! Find the nearest character at position and return it's index.
     */
    ssize_t character_index_at_position(point2 position) const noexcept;

    void set_cursor_at_coordinate(point2 coordinate) noexcept
    {
        tt_axiom(is_valid());
        if (ttlet new_cursor_position = _shaped_text.index_of_grapheme_at_coordinate(coordinate)) {
            _selection_index = _cursor_index = *new_cursor_position;
        }
        tt_axiom(is_valid());
    }

    void select_word_at_coordinate(point2 coordinate) noexcept
    {
        tt_axiom(is_valid());

        if (ttlet new_cursor_position = _shaped_text.index_of_grapheme_at_coordinate(coordinate)) {
            std::tie(_selection_index, _cursor_index) = _shaped_text.indices_of_word(*new_cursor_position);
        }

        tt_axiom(is_valid());
    }

    void select_paragraph_at_coordinate(point2 coordinate) noexcept
    {
        tt_axiom(is_valid());

        if (ttlet new_cursor_position = _shaped_text.index_of_grapheme_at_coordinate(coordinate)) {
            std::tie(_selection_index, _cursor_index) = _shaped_text.indices_of_paragraph(*new_cursor_position);
        }

        tt_axiom(is_valid());
    }

    void drag_cursor_at_coordinate(point2 coordinate) noexcept
    {
        tt_axiom(is_valid());

        if (ttlet new_cursor_position = _shaped_text.index_of_grapheme_at_coordinate(coordinate)) {
            _cursor_index = *new_cursor_position;
        }
        tt_axiom(is_valid());
    }

    void drag_word_at_coordinate(point2 coordinate) noexcept
    {
        tt_axiom(is_valid());

        if (ttlet new_cursor_position = _shaped_text.index_of_grapheme_at_coordinate(coordinate)) {
            ttlet[a, b] = _shaped_text.indices_of_word(*new_cursor_position);

            if (_selection_index <= _cursor_index) {
                if (a < _selection_index) {
                    // Reverse selection
                    _selection_index = _cursor_index;
                    _cursor_index = a;
                } else {
                    _cursor_index = b;
                }
            } else {
                if (b > _selection_index) {
                    // Reverse selection
                    _selection_index = _cursor_index;
                    _cursor_index = b;
                } else {
                    _cursor_index = a;
                }
            }
        }
        tt_axiom(is_valid());
    }

    void drag_paragraph_at_coordinate(point2 coordinate) noexcept
    {
        tt_axiom(is_valid());

        if (ttlet new_cursor_position = _shaped_text.index_of_grapheme_at_coordinate(coordinate)) {
            ttlet[a, b] = _shaped_text.indices_of_paragraph(*new_cursor_position);

            if (_selection_index <= _cursor_index) {
                if (a < _selection_index) {
                    // Reverse selection
                    _selection_index = _cursor_index;
                    _cursor_index = a;
                } else {
                    _cursor_index = b;
                }
            } else {
                if (b > _selection_index) {
                    // Reverse selection
                    _selection_index = _cursor_index;
                    _cursor_index = b;
                } else {
                    _cursor_index = a;
                }
            }
        }

        tt_axiom(is_valid());
    }

    void cancel_partial_grapheme() noexcept
    {
        tt_axiom(is_valid());

        if (_has_partial_grapheme) {
            tt_axiom(_cursor_index >= 1);

            _selection_index = --_cursor_index;

            _text.erase(cit(_cursor_index));
            _has_partial_grapheme = false;

            update_shaped_text();
        }

        tt_axiom(is_valid());
    }

    /*! Insert a temporary partial character.
     * This partial character is currently being constructed by the operating system.
     *
     * Since the insertion has not been completed any selected _text should not yet be deleted.
     */
    void insert_partial_grapheme(grapheme character) noexcept
    {
        tt_axiom(is_valid());

        cancel_partial_grapheme();
        delete_selection();

        _text.emplace_before(cit(_cursor_index), character, _current_style);
        _selection_index = ++_cursor_index;

        _has_partial_grapheme = true;
        update_shaped_text();

        tt_axiom(is_valid());
    }

    /*! insert character at the cursor position.
     * Selected _text will be deleted.
     */
    void insert_grapheme(grapheme character) noexcept
    {
        tt_axiom(is_valid());

        cancel_partial_grapheme();
        delete_selection();

        if (!_insert_mode) {
            handle_event(command::text_delete_char_next);
        }
        _text.emplace_before(cit(_cursor_index), character, _current_style);
        _selection_index = ++_cursor_index;

        update_shaped_text();

        tt_axiom(is_valid());
    }

    void handle_paste(std::string str) noexcept
    {
        tt_axiom(is_valid());

        cancel_partial_grapheme();
        delete_selection();

        gstring gstr = to_gstring(str);

        auto str_attr = std::vector<attributed_grapheme>{};
        str_attr.reserve(gstr.size());
        for (ttlet &g : gstr) {
            str_attr.emplace_back(g, _current_style);
        }

        _text.insert_after(cit(_cursor_index), str_attr.cbegin(), str_attr.cend());
        _selection_index = _cursor_index += ssize(str_attr);

        update_shaped_text();
        tt_axiom(is_valid());
    }

    std::string handle_copy() noexcept
    {
        tt_axiom(is_valid());

        auto r = std::string{};

        if (_selection_index < _cursor_index) {
            r.reserve(_cursor_index - _selection_index);
            for (auto i = cit(_selection_index); i != cit(_cursor_index); ++i) {
                r += to_string(i->grapheme);
            }
        } else if (_selection_index > _cursor_index) {
            r.reserve(_selection_index - _cursor_index);
            for (auto i = cit(_cursor_index); i != cit(_selection_index); ++i) {
                r += to_string(i->grapheme);
            }
        }

        tt_axiom(is_valid());
        return r;
    }

    std::string handle_cut() noexcept
    {
        tt_axiom(is_valid());

        auto r = handle_copy();
        cancel_partial_grapheme();
        delete_selection();

        tt_axiom(is_valid());
        return r;
    }

    bool handle_event(command command) noexcept
    {
        tt_axiom(is_valid());

        auto handled = false;
        switch (command) {
        case command::text_cursor_char_left:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfCharOnTheLeft(_cursor_index)) {
                // XXX Change _current_style based on the grapheme at the new cursor position.
                _selection_index = _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_cursor_char_right:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfCharOnTheRight(_cursor_index)) {
                _selection_index = _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_cursor_word_left:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfWordOnTheLeft(_cursor_index)) {
                _selection_index = _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_cursor_word_right:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfWordOnTheRight(_cursor_index)) {
                _selection_index = _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_cursor_line_end:
            cancel_partial_grapheme();
            handled = true;
            _selection_index = _cursor_index = size();
            break;

        case command::text_cursor_line_begin:
            cancel_partial_grapheme();
            handled = true;
            _selection_index = _cursor_index = 0;
            break;

        case command::text_select_char_left:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfCharOnTheLeft(_cursor_index)) {
                _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_select_char_right:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfCharOnTheRight(_cursor_index)) {
                _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_select_word_left:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfWordOnTheLeft(_cursor_index)) {
                _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_select_word_right:
            cancel_partial_grapheme();
            handled = true;
            if (ttlet new_cursor_position = _shaped_text.indexOfWordOnTheRight(_cursor_index)) {
                _cursor_index = *new_cursor_position;
            }
            break;

        case command::text_select_word:
            cancel_partial_grapheme();
            handled = true;
            std::tie(_selection_index, _cursor_index) = _shaped_text.indices_of_word(_cursor_index);
            break;

        case command::text_select_line_end:
            cancel_partial_grapheme();
            handled = true;
            _cursor_index = size();
            break;

        case command::text_select_line_begin:
            cancel_partial_grapheme();
            handled = true;
            _cursor_index = 0;
            break;

        case command::text_select_document:
            cancel_partial_grapheme();
            handled = true;
            _selection_index = 0;
            _cursor_index = size();
            break;

        case command::text_mode_insert:
            cancel_partial_grapheme();
            handled = true;
            _insert_mode = !_insert_mode;
            break;

        case command::text_delete_char_prev:
            cancel_partial_grapheme();
            handled = true;
            if (_cursor_index != _selection_index) {
                delete_selection();

            } else if (_cursor_index >= 1) {
                _selection_index = --_cursor_index;
                _text.erase(cit(_cursor_index));
                update_shaped_text();
            }
            break;

        case command::text_delete_char_next:
            cancel_partial_grapheme();
            handled = true;
            if (_cursor_index != _selection_index) {
                delete_selection();

            } else if (_cursor_index < ssize(_text)) {
                // Don't delete the trailing paragraph separator.
                _text.erase(cit(_cursor_index));
                update_shaped_text();
            }
        default:;
        }

        tt_axiom(is_valid());
        return handled;
    }

    bool is_valid() const noexcept
    {
        return _selection_index >= 0 && _selection_index <= ssize(_text) && _cursor_index >= 0 && _cursor_index <= ssize(_text);
    }

private:
    font_book const &_font_book;
    gap_buffer<attributed_grapheme> _text;
    tt::shaped_text _shaped_text;

    /** The maximum _width when wrapping _text.
     * For single line _text editing, we should never wrap.
     */
    float _width = 0.0f;

    /** Insert-mode vs overwrite-mode.
     */
    bool _insert_mode = true;

    /** The index into the _text where the cursor is located.
     */
    ssize_t _cursor_index = 0;

    /** The index into the _text where the start of the selection is located.
     * When no _text is selected the _cursor_index and _selection_index are equal.
     */
    ssize_t _selection_index = 0;

    text_style _current_style;

    /** Partial grapheme is inserted before _cursor_index.
     */
    bool _has_partial_grapheme = false;
};

} // namespace tt::inline v1
