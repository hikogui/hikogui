// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_widget.hpp"

namespace tt {

text_widget::~text_widget() {}

[[nodiscard]] bool text_widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        _shaped_text = shaped_text{text(), _style, 0.0f, _alignment};
        _minimum_size = _shaped_text.minimum_size();
        _preferred_size = _shaped_text.preferred_size();
        _maximum_size = _shaped_text.maximum_size();
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void text_widget::update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_layout |= std::exchange(this->_request_relayout, false);
    if (need_layout and visible()) {
        _shaped_text = shaped_text{text(), _style, width(), _alignment};
        _shaped_text_transform = _shaped_text.translate_base_line(point2{0.0f, base_line()});
    }
    super::update_layout(displayTimePoint, need_layout);
}

void text_widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (overlaps(context, _clipping_rectangle) and visible()) {
        context.draw_text(_shaped_text, this->label_color(), _shaped_text_transform);
    }

    super::draw(std::move(context), display_time_point);
}

}
