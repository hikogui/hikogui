// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GUI/theme_text_style.hpp"
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

namespace tt::inline v1 {

/** A text widget.
 * The text widget is a simple widget that just displays text.
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

        /** Text is editable.
         */
        editable
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
    struct undo_type {
        gstring text;
        text_selection selection;
    };

    text_shaper _shaped_text;
    float _shaped_text_cap_height;

    decltype(text)::callback_ptr_type _text_callback;

    text_selection _selection;
    bool _overwrite_mode = false;
    undo_stack<undo_type> _undo_stack = {1000};

    text_widget(gui_window &window, widget *parent) noexcept;
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
     * @param insert If true then the cursor remains at the current position.
     */
    void add_char(grapheme c, bool insert = false) noexcept;
    void delete_char_next() noexcept;
    void delete_char_prev() noexcept;
    void delete_word_next() noexcept;
    void delete_word_prev() noexcept;
};

} // namespace tt::inline v1
