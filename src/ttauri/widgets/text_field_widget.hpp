// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_field_delegate.hpp"
#include "default_text_field_delegate.hpp"
#include "widget.hpp"
#include "label_widget.hpp"
#include "scroll_widget.hpp"
#include "../format.hpp"
#include "../label.hpp"
#include "../weak_or_unique_ptr.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt::inline v1 {

/** A single line text field.
 *
 * A text field has the following visual elements:
 *  - A text field box which surrounds the user-editable text.
 *    It will use a color to show when the text-field has keyboard focus.
 *    It will use another color to show when the editable text is incorrect.
 *    Inside this box are the following elements:
 *     + Prefix: an icon describing the meaning, such as a search icon, or password, or popup-chevron.
 *     + Editable text
 *     + Suffix: text that follows the editable text, such as a SI base units like " kg" or " Hz".
 *  - Outside the text field box is an optional error message.
 *  - A popup window can be used to select between suggestions.
 *
 * Two commit modes:
 *  - on-activate: When pressing enter or changing keyboard focus using tab or clicking in another
 *                 field; as long as the text value can be validly converted.
 *                 The text will be converted to the observed object and committed.
 *                 When pressing escape, the text reverts to the observed object value.
 *  - continues: Every change of the text value of the input value is immediately converted and committed
 *               to the observed object; as long as the text value can be validly converted.
 *
 * The observed object needs to be convertible to and from a string using to_string() and from_string().
 * If from_string() throws a parse_error() its message will be displayed next to the text field.
 *
 * A custom validate function can be passed to validate the string and display a message next to the
 * text field.
 *
 * A custom transform function can be used to filter text on a modification-by-modification basis.
 * The filter takes the previous text and the new text after modification and returns the text that
 * should be shown in the field. This allows the filter to reject certain characters or limit the size.
 *
 * The maximum width of the text field is defined in the number of EM of the current selected font.
 */
class text_field_widget final : public widget {
public:
    using delegate_type = text_field_delegate;
    using super = widget;

    /** Continues update mode.
     * If true then the value will update on every edit of the text field.
     */
    observable<bool> continues = false;

    /** The style of the text.
     */
    observable<theme_text_style> text_style = theme_text_style::label;

    virtual ~text_field_widget();

    text_field_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept;

    template<typename Value>
    text_field_widget(gui_window &window, widget *parent, Value &&value) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        text_field_widget(window, parent, make_unique_default_text_field_delegate(std::forward<Value>(value)))
    {
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        co_yield _scroll_widget.get();
    }
    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &layout) noexcept override;
    void draw(draw_context const &context) noexcept override;
    bool handle_event(command command) noexcept override;
    hitbox hitbox_test(point3 position) const noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    [[nodiscard]] color focus_color() const noexcept override;
    /// @endprivatesection
private:
    weak_or_unique_ptr<delegate_type> _delegate;

    /** The scroll widget embeds the text widget.
     */
    std::unique_ptr<scroll_widget<axis::none, false>> _scroll_widget;

    /** The text widget inside the scroll widget.
     */
    text_widget *_text_widget = nullptr;

    /** The text edited by the _text_widget.
     */
    observable<gstring> _text;

    /** The rectangle where the text is displayed.
     */
    aarectangle _text_rectangle;

    /** An error string to show to the user.
     */
    observable<label> _error_label;
    aarectangle _error_label_rectangle;
    std::unique_ptr<label_widget> _error_label_widget;

    text_field_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept;
    void revert(bool force) noexcept;
    void commit(bool force) noexcept;
    void draw_background_box(draw_context const &context) const noexcept;
};

} // namespace tt::inline v1
