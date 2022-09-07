// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "text_widget.hpp"
#include "icon_widget.hpp"
#include "../alignment.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi::inline v1 {

template<typename Context>
concept label_widget_attribute =
    forward_of<Context, observer<hi::label>, observer<hi::alignment>, observer<hi::semantic_text_style>>;

/** The GUI widget displays and lays out text together with an icon.
 *
 * This widget is often used by other widgets. For example
 * checkboxes display a label representing their state next
 * to the checkbox itself.
 *
 * The alignment of icon and text is shown in the following image:
 * @image html label_widget.png
 *
 * Here is an example on how to create a label:
 * @snippet widgets/checkbox_example.cpp Create a label
 */
class label_widget final : public widget {
public:
    using super = widget;

    /** The label to display.
     */
    observer<label> label;

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

    /** Construct a label widget.
     *
     * @see `label_widget::alignment`
     * @param window The window that this widget belongs to.
     * @param parent The parent widget that owns this radio button widget.
     * @param label The label to show next to the radio button.
     * @param alignment The alignment of the label.
     *                  The default alignment is middle_right, because the most common
     *                  usage for a label by an application programmer is to add the label
     *                  to the left of another widget.
     * @param text_style The text style of the label, and color of non-color
     *                   icons.
     */
    label_widget(
        gui_window& window,
        widget *parent,
        label_widget_attribute auto&&...attributes) noexcept :
        label_widget(window, parent)
    {
        set_attributes(hi_forward(attributes)...);
    }

    void set_attributes() noexcept {}
    void set_attributes(
        label_widget_attribute auto&& first, label_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            label = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::text_style>>) {
            text_style = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        co_yield _icon_widget.get();
        co_yield _text_widget.get();
    }

    widget_constraints const& set_constraints() noexcept override;
    void set_layout(widget_layout const& layout) noexcept override;
    void draw(draw_context const& context) noexcept;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept;
    /// @endprivatesection
private:
    float _icon_size;
    float _inner_margin;

    decltype(label)::token_type _label_cbt;
    decltype(text_style)::token_type _text_style_cbt;

    aarectangle _icon_rectangle;
    widget_constraints _icon_constraints;
    std::unique_ptr<icon_widget> _icon_widget;
    aarectangle _text_rectangle;
    widget_constraints _text_constraints;
    std::unique_ptr<text_widget> _text_widget;

    label_widget(gui_window& window, widget *parent) noexcept;
};

} // namespace hi::inline v1
