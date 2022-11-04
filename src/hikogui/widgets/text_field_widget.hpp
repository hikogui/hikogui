// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/text_field_widget.hpp Defines text_field_widget.
 * @ingroup widgets
 */

#pragma once

#include "text_field_delegate.hpp"
#include "widget.hpp"
#include "label_widget.hpp"
#include "scroll_widget.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept text_field_widget_attribute = text_widget_attribute<Context>;

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
 *
 * @ingroup widgets
 */
class text_field_widget final : public widget {
public:
    using delegate_type = text_field_delegate;
    using super = widget;

    std::shared_ptr<delegate_type> delegate;

    /** Continues update mode.
     * If true then the value will update on every edit of the text field.
     */
    observer<bool> continues = false;

    /** The style of the text.
     */
    observer<semantic_text_style> text_style = semantic_text_style::label;

    /** The alignment of the text.
     */
    observer<alignment> alignment = alignment::middle_flush();

    virtual ~text_field_widget();

    text_field_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept;

    text_field_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        text_field_widget_attribute auto&&...attributes) noexcept :
        text_field_widget(parent, std::move(delegate))
    {
        set_attributes(hi_forward(attributes)...);
    }

    /** Construct a text field widget.
     *
     * @param parent The owner of this widget.
     * @param value The value or `observer` value which represents the state of the text-field.
     * @param attributes A set of attributes used to configure the text widget: a `alignment` or `semantic_text_style`.
     */
    text_field_widget(
        widget *parent,
        different_from<std::shared_ptr<delegate_type>> auto&& value,
        text_field_widget_attribute auto&&...attributes) noexcept requires requires
    {
        make_default_text_field_delegate(hi_forward(value));
    } : text_field_widget(parent, make_default_text_field_delegate(hi_forward(value)), hi_forward(attributes)...) {}

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override;
    widget_constraints const& set_constraints(set_constraints_context const &context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    bool handle_event(gui_event const& event) noexcept override;
    hitbox hitbox_test(point3 position) const noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    [[nodiscard]] color focus_color() const noexcept override;
    /// @endprivatesection
private:
    notifier<>::callback_token _delegate_cbt;

    /** The scroll widget embeds the text widget.
     */
    std::unique_ptr<scroll_widget<axis::none>> _scroll_widget;

    /** The text widget inside the scroll widget.
     */
    text_widget *_text_widget = nullptr;

    /** The text edited by the _text_widget.
     */
    observer<gstring> _text;

    /** The rectangle where the box is displayed, in which the text is displayed.
     */
    aarectangle _box_rectangle;

    /** The rectangle where the text is displayed.
     */
    aarectangle _text_rectangle;
    widget_constraints _text_constraints;

    /** An error string to show to the user.
     */
    observer<label> _error_label;
    aarectangle _error_label_rectangle;
    widget_constraints _error_label_constraints;
    std::unique_ptr<label_widget> _error_label_widget;

    typename decltype(continues)::callback_token _continues_cbt;
    typename decltype(text_style)::callback_token _text_style_cbt;
    typename decltype(_text)::callback_token _text_cbt;
    typename decltype(_error_label)::callback_token _error_label_cbt;

    void set_attributes() noexcept {}
    void set_attributes(text_field_widget_attribute auto&& first, text_field_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::semantic_text_style>>) {
            text_style = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
    }

    void revert(bool force) noexcept;
    void commit(bool force) noexcept;
    void draw_background_box(draw_context const& context) const noexcept;
};

}} // namespace hi::v1
