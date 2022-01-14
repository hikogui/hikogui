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

    _text_shaper = text_shaper{font_book(), (*text)(), theme().text_style(*text_style)};
    ttlet[shaped_text_rectangle, x_height] = _text_shaper.bounding_rectangle(500.0f, alignment->vertical());
    _text_shaper_x_height = x_height;
    ttlet shaped_text_size = shaped_text_rectangle.size();
    _constraints = {shaped_text_size, shaped_text_size, shaped_text_size, theme().margin};

    tt_axiom(_constraints.holds_invariant());
    return _constraints;
}

void text_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _text_shaper.layout(
            layout.rectangle(),
            layout.base_line() - _text_shaper_x_height * 0.5f,
            layout.sub_pixel_size,
            layout.writing_direction,
            *alignment);
    }
}

void text_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        context.draw_text(layout(), _text_shaper);
    }
}

} // namespace tt::inline v1
