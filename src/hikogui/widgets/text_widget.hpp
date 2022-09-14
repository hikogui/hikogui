// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "text_delegate.hpp"
#include "default_text_delegate.hpp"
#include "../GUI/gui_event.hpp"
#include "../text/semantic_text_style.hpp"
#include "../text/text_selection.hpp"
#include "../text/text_shaper.hpp"
#include "../observer.hpp"
#include "../alignment.hpp"
#include "../i18n/translate.hpp"
#include "../undo_stack.hpp"
#include "../scoped_task.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>
#include <limits>
#include <chrono>

namespace hi::inline v1 {

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
    using delegate_type = text_delegate;

    std::shared_ptr<delegate_type> delegate;

    /** The horizontal alignment of the text inside the space of the widget.
     */
    observer<alignment> alignment = hi::alignment::middle_center();

    /** The style of the text.
     */
    observer<semantic_text_style> text_style = semantic_text_style::label;

    ~text_widget();

    /** Construct a text widget.
     *
     * @param window The window the widget is displayed on.
     * @param parent The owner of this widget.
     * @param delegate The delegate to use to control the widget's data.
     */
    text_widget(gui_window& window, widget *parent, std::shared_ptr<delegate_type> delegate) noexcept;

    /** Construct a text widget.
     *
     * @param window The window the widget is displayed on.
     * @param parent The owner of this widget.
     * @param text The text to be displayed.
     * @param alignment The alignment of the text inside the space of the widget.
     * @param text_style The style of the text to be displayed.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Text,
        forward_of<observer<hi::alignment>> Alignment = hi::alignment,
        forward_of<observer<semantic_text_style>> TextStyle = semantic_text_style>
    text_widget(
        gui_window& window,
        widget *parent,
        Text&& text,
        Alignment&& alignment = hi::alignment::middle_center(),
        TextStyle&& text_style = semantic_text_style::label) noexcept requires requires
    {
        make_default_text_delegate(std::forward<Text>(text));
    } : text_widget(window, parent, make_default_text_delegate(std::forward<Text>(text)))
    {
        this->alignment = std::forward<Alignment>(alignment);
        this->text_style = std::forward<TextStyle>(text_style);
    }

    /// @privatesection
    widget_constraints const& set_constraints() noexcept override;
    void set_layout(widget_layout const& layout) noexcept override;
    void draw(draw_context const& context) noexcept override;
    bool handle_event(gui_event const& event) noexcept override;
    hitbox hitbox_test(point3 position) const noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    /// @endprivatesection
private:
    enum class add_type { append, insert, dead };

    struct undo_type {
        gstring text;
        text_selection selection;
    };

    enum class cursor_state_type { off, on, busy, none };

    gstring _cached_text;
    text_shaper _shaped_text;
    float _base_line;

    delegate_type::callback_token _delegate_cbt;

    decltype(text_style)::callback_token _text_style_cbt;

    text_selection _selection;

    scoped_task<> _blink_cursor;

    observer<cursor_state_type> _cursor_state = cursor_state_type::none;
    decltype(_cursor_state)::callback_token _cursor_state_cbt;

    /** After layout request scroll from the parent widgets.
     */
    bool _request_scroll = false;

    /** The last drag mouse event.
     *
     * This variable is used to repeatably execute the mouse event
     * even in absent of new mouse events. This must be done to get
     * continues scrolling to work during dragging.
     */
    gui_event _last_drag_mouse_event = {};

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
    grapheme _has_dead_character = nullptr;

    undo_stack<undo_type> _undo_stack = {1000};

    /** Update the shaped text.
     *
     * This function must be called synchronously whenever the text, style or theme changes.
     */
    void update_shaped_text() noexcept;

    /** Make parent scroll views, scroll to show the current selection and cursor.
     */
    void scroll_to_show_selection() noexcept;

    void request_scroll() noexcept;

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

    scoped_task<> blink_cursor() noexcept;

    /** Fix the cursor position after cursor movement.
     */
    void fix_cursor_position() noexcept;

    void replace_selection(gstring const& replacement) noexcept;

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

} // namespace hi::inline v1
