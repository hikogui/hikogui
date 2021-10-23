// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"

namespace tt {

text_widget::text_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    text.subscribe(_reconstrain_callback);
}

widget_constraints const &text_widget::set_constraints() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};

    _shaped_text = shaped_text{font_book(), (*text)(), theme().text_style(*text_style), 0.0f, *alignment};
    _constraints.min = ceil(_shaped_text.preferred_size());
    _constraints.pref = ceil(_shaped_text.preferred_size());
    _constraints.max = ceil(_shaped_text.preferred_size());

    ttlet size_ = theme().size;
    ttlet margin_ = margin();

    // Allow text to overhang into the margin of a small widget.
    if (_constraints.min.height() > size_ && _constraints.min.height() <= size_ + margin_) {
        _constraints.min.height() = size_;
    }
    if (_constraints.pref.height() > size_ && _constraints.pref.height() <= size_ + margin_) {
        _constraints.pref.height() = size_;
    }
    if (_constraints.max.height() > size_ && _constraints.max.height() <= size_ + margin_) {
        _constraints.max.height() = size_;
    }

    tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);
    return _constraints;
}

void text_widget::set_layout(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and _layout.store(context) >= layout_update::size) {
        _shaped_text = shaped_text{font_book(), (*text)(), theme().text_style(*text_style), layout().width(), *alignment};
        _shaped_text_transform = _shaped_text.translate_base_line(point2{0.0f, layout().base_line()});
    }
}

void text_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        context.draw_text(layout(), _shaped_text, label_color(), _shaped_text_transform);
    }
}

} // namespace tt
