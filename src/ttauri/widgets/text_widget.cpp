// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"

namespace tt::inline v1 {

text_widget::text_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    text.subscribe(_reconstrain_callback);
}

widget_constraints const &text_widget::set_constraints() noexcept
{
    _layout = {};

    _selection_first = 0;
    _selection_last = 0;
    _shaped_text = text_shaper{font_book(), *text, theme().text_style(*text_style)};
    ttlet[shaped_text_rectangle, cap_height] = _shaped_text.bounding_rectangle(500.0f, alignment->vertical());
    _shaped_text_cap_height = cap_height;
    ttlet shaped_text_size = shaped_text_rectangle.size();

    return _constraints = {shaped_text_size, shaped_text_size, shaped_text_size, theme().margin};
    ;
}

void text_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _shaped_text.layout(
            layout.rectangle(),
            layout.base_line() - _shaped_text_cap_height * 0.5f,
            layout.sub_pixel_size,
            layout.writing_direction,
            *alignment);
    }
}

void text_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        context.draw_text(layout(), _shaped_text);
        context.draw_text_selection(
            layout(), _shaped_text, _selection_first, _selection_last, theme().color(theme_color::text_select));
    }
}

} // namespace tt::inline v1
