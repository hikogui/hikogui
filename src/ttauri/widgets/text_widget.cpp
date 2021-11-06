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

    _shaped_text = shaped_text{font_book(), (*text)(), theme().text_style(*text_style), 0.0f, *alignment};
    ttlet shaped_text_size = ceil(_shaped_text.preferred_size());
    _constraints = {shaped_text_size, shaped_text_size, shaped_text_size, theme().margin};

    // Allow text to overhang into the margin of a small widget.
    if (_constraints.minimum.height() > theme().size and _constraints.minimum.height() <= theme().size + theme().margin) {
        _constraints.minimum.height() = theme().size;
    }
    if (_constraints.preferred.height() > theme().size and _constraints.preferred.height() <= theme().size + theme().margin) {
        _constraints.preferred.height() = theme().size;
    }
    if (_constraints.maximum.height() > theme().size and _constraints.maximum.height() <= theme().size + theme().margin) {
        _constraints.maximum.height() = theme().size;
    }

    tt_axiom(_constraints.holds_invariant());
    return _constraints;
}

void text_widget::set_layout(widget_layout const &context) noexcept
{
    if (_layout.store(context) >= layout_update::size) {
        _shaped_text = shaped_text{font_book(), (*text)(), theme().text_style(*text_style), layout().width(), *alignment};
        _shaped_text_transform = _shaped_text.translate_base_line(point2{0.0f, layout().base_line()});
    }
}

void text_widget::draw(draw_context const &context) noexcept
{
    if (visible and overlaps(context, layout())) {
        context.draw_text(layout(), _shaped_text_transform, _shaped_text);
    }
}

} // namespace tt::inline v1
