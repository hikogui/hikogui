// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GUI/theme_text_style.hpp"
#include "../GUI/mouse_event.hpp"
#include "../text/text_selection.hpp"
#include "../text/text_shaper.hpp"
#include "../observable.hpp"
#include "../alignment.hpp"
#include "../l10n.hpp"
#include "../undo_stack.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>
#include <limits>
#include <chrono>

namespace tt::inline v1 {

/** A text widget.
 *
 * The text widget is a widget for displaying, selecting and editing text.
 *
 * On its own it can be used to edit multiple lines of text, but it will probably
 * be used embedded inside other widgets, like:
 *  - `label_widget` to display translated text together with an optional icon.
 *  - `text_field_widget` to edit a value of diffent types, includig integers, floating point, strings, etc.
 *
 * Features:
 *  - Multiple paragraphs.
 *  - Uses the unicode line break algorithm to wrap lines when not enough horizontal space.
 *  - Used the unicode word break algorithm for selecting and moving through words.
 *  - Uses the unicode scentence break algorithm for selecting and moving through scentences.
 *  - Uses the unicode bidi algorithm for displaying text in mixed left-to-right & right-to-left languages.
 *  - Displays secondary cursor where text in the other language-direction will be inserted.
 *  - Keeps track if the user has just worked in left-to-right or right-to-left language.
 *  - Arrow keys move the cursor visually through the text
 *  - Handles insertion and overwrite mode; showing a caret or box cursor.
 *  - When entering dead-key on the keyboard the dead-key character is displayed underneath a secondary
 *    overwrite cursor.
 *  - Cut, Copy & Paste.
 *  - Undo & Redo.
 *
 */
class text_widget final : public widget {
public:
    using super = widget;

    /** Mode of the text-widget.
     */
    enum class edit_mode_type {
        /** Text is fixed.
         */
        fixed,

        /** Text is selectable and copyable.
         */
        selectable,

        /** Can edit single lines.
         */
        line_editable,

        /** Text is editable.
         */
        fully_editable
    };

    /** The text to be displayed.
     */
    observable<gstring> text;

    /** The horizontal alignment of the text inside the space of the widget.
     */
    observable<alignment> alignment = tt::alignment::middle_center();

    /** The style of the text.
     */
    observable<theme_text_style> text_style = theme_text_style::label;

    /** The edit-mode.
     */
    observable<edit_mode_type> edit_mode = edit_mode_type::selectable;

    /** Construct a text widget.
     *
     * @param window The window the widget is displayed on.
     * @param parent The owner of this widget.
     * @param text The text to be displayed.
     * @param horizontal_alignment The horizontal alignment of the text inside the space of the widget.
     * @param vertical_alignment The vertical alignment of the text inside the space of the widget.
     * @param text_style The style of the text to be displayed.
     */
    template<
        typename Text,
        typename Alignment = tt::alignment,
        typename VerticalAlignment = tt::vertical_alignment,
        typename TextStyle = tt::theme_text_style>
    text_widget(
        gui_window &window,
        widget *parent,
        Text &&text,
        Alignment &&alignment = tt::alignment::middle_center(),
        TextStyle &&text_style = theme_text_style::label) noexcept :
        text_widget(window, parent)
    {
        this->text = std::forward<Text>(text);
        this->alignment = std::forward<Alignment>(alignment);
        this->text_style = std::forward<TextStyle>(text_style);
    }

    /// @privatesection
    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &layout) noexcept override;
    void draw(draw_context const &context) noexcept override;
    bool handle_event(tt::command command) noexcept override;
    bool handle_event(keyboard_event const &event) noexcept override;
    bool handle_event(mouse_event const &event) noexcept override;
    hitbox hitbox_test(point3 position) const noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    /// @endprivatesection
private:
    enum class add_type { append, insert, dead };

    struct undo_type {
        gstring text;
        text_selection selection;
    };

    text_shaper _shaped_text;
    float _shaped_text_cap_height;

    decltype(text)::callback_ptr_type _text_callback;

    text_selection _selection;

    utc_nanoseconds _cursor_blink_time_point = {};


    /** The last drag mouse event.
     *
     * This variable is used to repeatably execute the mouse event
     * even in absent of new mouse events. This must be done to get
     * continues scrolling to work during dragging.
     */
    mouse_event _last_drag_mouse_event = {};

    /** When to cause the next mouse drag event repeat.
     */
    utc_nanoseconds _last_drag_mouse_event_next_repeat = {};

    /** The x-coordinate during vertical movement.
     */
    float _vertical_movement_x = std::numeric_limits<float>::quiet_NaN();

    bool _overwrite_mode = false;

    /** The text has a dead character.
     * The grapheme is empty when there is no dead character.
     * On overwrite the original grapheme is stored in the _had_dead_character, so
     * that it can be restored.
     */
    grapheme _has_dead_character = {};

    undo_stack<undo_type> _undo_stack = {1000};

    text_widget(gui_window &window, widget *parent) noexcept;

    /** Make parent scroll views, scroll to show the current selection and cursor.
     */
    void scroll_to_show_selection() noexcept;

    /** Reset states.
     *
     * Possible states:
     *  - 'X' x-coordinate for vertical movement.
     *  - 'D' Dead-character state.
     *  - 'B' Reset cursor blink time.
     *
     * @param states The individual states to reset.
     */
    void reset_state(char const *states) noexcept;

    [[nodiscard]] gstring_view selected_text() const noexcept;
    void undo_push() noexcept;
    void undo() noexcept;
    void redo() noexcept;

    /** Fix the cursor position after cursor movement.
     *
     * @param Size of the text.
     */
    void fix_cursor_position(size_t size) noexcept;

    /** Fix the cursor position after cursor movement.
     *
     * @note uses size of _shaped_text.
     */
    void fix_cursor_position() noexcept
    {
        fix_cursor_position(_shaped_text.size());
    }

    void replace_selection(gstring replacement) noexcept;

    /** Add a character to the text.
     *
     * @param c The character to add at the current position
     * @param mode The mode how to add a character.
     */
    void add_character(grapheme c, add_type mode) noexcept;
    void delete_dead_character() noexcept;
    void delete_character_next() noexcept;
    void delete_character_prev() noexcept;
    void delete_word_next() noexcept;
    void delete_word_prev() noexcept;
};

} // namespace tt::inline v1
