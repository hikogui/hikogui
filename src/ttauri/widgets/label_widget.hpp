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

namespace tt {

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
    observable<label> label;

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
    observable<alignment> alignment = alignment::middle_right;

    /** The text style to display the label's text in and color of the label's (non-color) icon.
     */
    observable<theme_text_style> text_style = theme_text_style::label;

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
    template<typename Label, typename Alignment = tt::alignment, typename TextStyle = tt::theme_text_style>
    label_widget(
        gui_window &window,
        widget *parent,
        Label &&label,
        Alignment &&alignment = alignment::middle_right,
        TextStyle &&text_style = theme_text_style::label) noexcept :
        label_widget(window, parent)
    {
        this->label = std::forward<Label>(label);
        this->alignment = std::forward<Alignment>(alignment);
        this->text_style = std::forward<TextStyle>(text_style);
    }

    /// @privatesection
    [[nodiscard]] bool constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void layout(utc_nanoseconds displayTimePoint, bool need_layout) noexcept override;
    /// @endprivatesection
private:
    float _icon_size;
    float _inner_margin;

    decltype(label)::callback_ptr_type _label_callback;

    icon_widget *_icon_widget = nullptr;
    text_widget *_text_widget = nullptr;

    label_widget(gui_window &window, widget *parent) noexcept;
};

} // namespace tt
