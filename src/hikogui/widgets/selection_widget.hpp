// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/selection_widget.hpp Defines selection_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "label_widget.hpp"
#include "overlay_widget.hpp"
#include "scroll_widget.hpp"
#include "row_column_widget.hpp"
#include "menu_button_widget.hpp"
#include "selection_delegate.hpp"
#include "../observer.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept selection_widget_attribute = label_widget_attribute<Context>;

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive options.
 *
 * @image html selection_widget.gif
 *
 * The following example creates a selection widget with three options.
 * which will monitor and modify `value` to display the options from
 * the `option_list`. At application startup, value is zero and none
 * of the options is selected:
 *
 * @snippet widgets/selection_example.cpp Create selection
 *
 * @ingroup widgets
 */
class selection_widget final : public widget {
public:
    using super = widget;
    using delegate_type = selection_delegate;

    std::shared_ptr<delegate_type> delegate;

    /** The label to show when nothing is selected.
     */
    observer<label> off_label;

    /** How the label and icon are aligned. Different layouts:
     *  - `alignment::top_left`: icon and text are inline with each other, with
     *    the icon in the top-left corner.
     *  - `alignment::top_right`: icon and text are inline with each other, with
     *    the icon in the top-right corner.
     *  - `alignment::middle_left`: icon and text are inline with each other, with
     *    the icon in the middle-left.
     *  - `alignment::middle_right`: icon and text are inline with each other, with
     *    the icon in the middle-right.
     *  - `alignment::bottom_left`: icon and text are inline with each other, with
     *    the icon in the bottom-left.
     *  - `alignment::bottom_right`: icon and text are inline with each other, with
     *    the icon in the bottom-right.
     *  - `alignment::top_center`: Larger icon above the text, both center aligned.
     *  - `alignment::bottom_center`: Larger icon below the text, both center aligned.
     *  - `alignment::middle_center`: text drawn across a large icon. Should only be
     *    used with a `pixmap` icon.
     */
    observer<alignment> alignment = hi::alignment::top_right();

    /** The text style to display the label's text in and color of the label's (non-color) icon.
     */
    observer<semantic_text_style> text_style = semantic_text_style::label;

    ~selection_widget();

    /** Construct a selection widget with a delegate.
     *
     * @param parent The owner of the selection widget.
     * @param delegate The delegate which will control the selection widget.
     */
    selection_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept;

    /** Construct a selection widget with a delegate.
     *
     * @param parent The owner of the selection widget.
     * @param delegate The delegate which will control the selection widget.
     * @param first_attribute First of @a attributes.
     * @param attributes Different attributes used to configure the label's on the selection box:
     *                   a `label`, `alignment` or `semantic_text_style`. If an label is passed
     *                   it is used as the label to show in the off-state.
     */
    selection_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        selection_widget_attribute auto&& first_attribute,
        selection_widget_attribute auto&&...attributes) noexcept :
        selection_widget(parent, std::move(delegate))
    {
        set_attributes(hi_forward(first_attribute), hi_forward(attributes)...);
    }

    /** Construct a selection widget which will monitor an option list and a
     * value.
     *
     * @param parent The owner of the selection widget.
     * @param value The value or observer value to monitor.
     * @param option_list An vector or an observer vector of pairs of keys and
     *                    labels. The keys are of the same type as the @a value.
     *                    The labels are of type `label`.
     * @param attributes Different attributes used to configure the label's on the selection box:
     *                   a `label`, `alignment` or `semantic_text_style`. If an label is passed
     *                   it is used as the label to show in the off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<std::vector<std::pair<observer_decay_t<Value>, label>>>> OptionList,
        selection_widget_attribute... Attributes>
    selection_widget(
        widget *parent,
        Value&& value,
        OptionList&& option_list,
        Attributes&&...attributes) noexcept requires requires
    {
        make_default_selection_delegate(hi_forward(value), hi_forward(option_list));
    } :
        selection_widget(
            parent,
            make_default_selection_delegate(hi_forward(value), hi_forward(option_list)),
            hi_forward(attributes)...)
    {
    }

    /** Construct a selection widget which will monitor an option list and a
     * value.
     *
     * @param parent The owner of the selection widget.
     * @param value The value or observer value to monitor.
     * @param option_list An vector or an observer vector of pairs of keys and
     *                    labels. The keys are of the same type as the @a value.
     *                    The labels are of type `label`.
     * @param off_value An optional off-value. This value is used to determine which
     *             value yields an 'off' state.
     * @param attributes Different attributes used to configure the label's on the selection box:
     *                   a `label`, `alignment` or `semantic_text_style`. If an label is passed
     *                   it is used as the label to show in the off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<std::vector<std::pair<observer_decay_t<Value>, label>>>> OptionList,
        forward_of<observer<observer_decay_t<Value>>> OffValue,
        selection_widget_attribute... Attributes>
    selection_widget(
        widget *parent,
        Value&& value,
        OptionList&& option_list,
        OffValue&& off_value,
        Attributes&&...attributes) noexcept requires requires
    {
        make_default_selection_delegate(hi_forward(value), hi_forward(option_list), hi_forward(off_value));
    } :
        selection_widget(
            parent,
            make_default_selection_delegate(hi_forward(value), hi_forward(option_list), hi_forward(off_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override;
    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    bool handle_event(gui_event const& event) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    [[nodiscard]] color focus_color() const noexcept override;
    /// @endprivatesection
private:
    notifier<>::callback_token _delegate_cbt;
    std::atomic<bool> _notification_from_delegate = true;

    std::unique_ptr<label_widget> _current_label_widget;
    std::unique_ptr<label_widget> _off_label_widget;

    aarectangle _option_rectangle;
    aarectangle _left_box_rectangle;

    glyph_ids _chevrons_glyph;
    aarectangle _chevrons_rectangle;

    bool _selecting = false;
    bool _has_options = false;

    aarectangle _overlay_rectangle;
    std::unique_ptr<overlay_widget> _overlay_widget;
    vertical_scroll_widget *_scroll_widget = nullptr;
    column_widget *_column_widget = nullptr;

    decltype(off_label)::callback_token _off_label_cbt;
    std::vector<menu_button_widget *> _menu_button_widgets;
    std::vector<notifier<>::callback_token> _menu_button_tokens;

    void set_attributes() noexcept {}
    void set_attributes(label_widget_attribute auto&& first, label_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            off_label = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::semantic_text_style>>) {
            text_style = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
    }

    [[nodiscard]] menu_button_widget const *get_first_menu_button() const noexcept;
    [[nodiscard]] menu_button_widget const *get_selected_menu_button() const noexcept;
    void start_selecting() noexcept;
    void stop_selecting() noexcept;
    void repopulate_options() noexcept;
    void draw_outline(draw_context const& context) noexcept;
    void draw_left_box(draw_context const& context) noexcept;
    void draw_chevrons(draw_context const& context) noexcept;
};

}} // namespace hi::v1
