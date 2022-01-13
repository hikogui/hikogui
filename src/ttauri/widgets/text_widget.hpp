// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GUI/theme_text_style.hpp"
#include "../text/shaped_text.hpp"
#include "../text/text_shaper.hpp"
#include "../observable.hpp"
#include "../alignment.hpp"
#include "../l10n.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt::inline v1 {

/** A text widget.
 * The text widget is a simple widget that just displays text.
 */
class text_widget final : public widget {
public:
    using super = widget;

    /** The text to be displayed.
     */
    observable<l10n> text;

    /** The horizontal alignment of the text inside the space of the widget.
     */
    observable<alignment> alignment = tt::alignment{horizontal_alignment::center, vertical_alignment::middle};

    /** The style of the text.
     */
    observable<theme_text_style> text_style = theme_text_style::label;

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
        Alignment &&alignment = tt::alignment{horizontal_alignment::center, vertical_alignment::middle},
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
    /// @endprivatesection
private:
    text_shaper _text_shaper;
    float _text_shaper_x_height;

    text_widget(gui_window &window, widget *parent) noexcept;
};

} // namespace tt::inline v1
