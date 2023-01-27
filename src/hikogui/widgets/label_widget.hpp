// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/label_widget.hpp Defines label_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "text_widget.hpp"
#include "icon_widget.hpp"
#include "../geometry/module.hpp"
#include "../layout/grid_layout.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept label_widget_attribute =
    forward_of<Context, observer<hi::label>, observer<hi::alignment>, observer<hi::semantic_text_style>>;

/** The GUI widget displays and lays out text together with an icon.
 * @ingroup widgets
 *
 * This widget is often used by other widgets. For example
 * checkboxes display a label representing their state next
 * to the checkbox itself.
 *
 * The alignment of icon and text is shown in the following image:
 * @image html label_widget.png
 *
 * Here is an example on how to create a label:
 * @snippet widgets/checkbox_example_impl.cpp Create a label
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
     *    used with a `pixmap` icon.
     */
    observer<alignment> alignment = hi::alignment::top_flush();

    /** The text style to display the label's text in and color of the label's (non-color) icon.
     */
    observer<semantic_text_style> text_style = semantic_text_style::label;

    /** Construct a label widget.
     *
     * @see `label_widget::alignment`
     * @param parent The parent widget that owns this radio button widget.
     * @param attributes Different attributes used to configure the label widget:
     *                   a `label`, `alignment` or `text_style`
     */
    label_widget(widget *parent, label_widget_attribute auto&&...attributes) noexcept : label_widget(parent)
    {
        set_attributes(hi_forward(attributes)...);
    }

    /// @privatesection
    [[nodiscard]] generator<widget const &> children(bool include_invisible) const noexcept override
    {
        co_yield *_icon_widget;
        co_yield *_text_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override;
    /// @endprivatesection
private:
    float _icon_size;
    float _inner_margin;

    decltype(label)::callback_token _label_cbt;
    decltype(text_style)::callback_token _text_style_cbt;
    decltype(alignment)::callback_token _alignment_cbt;

    std::unique_ptr<icon_widget> _icon_widget;
    std::unique_ptr<text_widget> _text_widget;
    grid_layout<widget *> _grid;

    void set_attributes() noexcept {}
    void set_attributes(label_widget_attribute auto&& first, label_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            label = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::semantic_text_style>>) {
            text_style = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
    }

    label_widget(widget *parent) noexcept;
};

}} // namespace hi::v1
