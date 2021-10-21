// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"

namespace tt {

text_widget::text_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    text.subscribe(_reconstrain_callback);
}

void text_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};

    _shaped_text = shaped_text{font_book(), (*text)(), theme().text_style(*text_style), 0.0f, *alignment};
    _minimum_size = ceil(_shaped_text.minimum_size());
    _preferred_size = ceil(_shaped_text.preferred_size());
    _maximum_size = ceil(_shaped_text.maximum_size());

    ttlet size_ = theme().size;
    ttlet margin_ = margin();

    // Allow text to overhang into the margin of a small widget.
    if (_minimum_size.height() > size_ && _minimum_size.height() <= size_ + margin_) {
        _minimum_size.height() = size_;
    }
    if (_preferred_size.height() > size_ && _preferred_size.height() <= size_ + margin_) {
        _preferred_size.height() = size_;
    }
    if (_maximum_size.height() > size_ && _maximum_size.height() <= size_ + margin_) {
        _maximum_size.height() = size_;
    }

    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void text_widget::layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and _layout.store(context) >= layout_update::size) {
        _shaped_text = shaped_text{font_book(), (*text)(), theme().text_style(*text_style), width(), *alignment};
        _shaped_text_transform = _shaped_text.translate_base_line(point2{0.0f, base_line()});
    }
}

void text_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, _layout)) {
        context.draw_text(_layout, _shaped_text, label_color(), _shaped_text_transform);
    }
}

} // namespace tt
