// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "overlay_widget.hpp"
#include "../GUI/theme.hpp"

namespace tt {

overlay_widget::overlay_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    super(window, parent), _delegate(std::move(delegate))
{
    tt_axiom(is_gui_thread());

    if (parent) {
        // The overlay-widget will reset the semantic_layer as it is the bottom
        // layer of this virtual-window. However the draw-layer should be above
        // any other widget drawn.
        draw_layer = parent->draw_layer + 20.0f;
        semantic_layer = 0;
    }
}

void overlay_widget::init() noexcept
{
    super::init();
    if (auto delegate = _delegate.lock()) {
        delegate->init(*this);
    }
}

void overlay_widget::deinit() noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
    super::deinit();
}

[[nodiscard]] bool
overlay_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    auto has_updated_contraints = super::constrain(display_time_point, need_reconstrain);

    if (has_updated_contraints) {
        tt_axiom(_content);
        _minimum_size = _content->minimum_size();
        _preferred_size = _content->preferred_size();
        _maximum_size = _content->maximum_size();
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
    }

    return has_updated_contraints;
}

[[nodiscard]] void overlay_widget::layout(utc_nanoseconds display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        tt_axiom(_content);
        _content->set_layout_parameters_from_parent(rectangle(), rectangle(), 1.0f);
    }

    super::layout(display_time_point, need_layout);
}

void overlay_widget::draw(draw_context context, utc_nanoseconds display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, _clipping_rectangle)) {
        draw_background(context);
    }

    super::draw(std::move(context), display_time_point);
}

[[nodiscard]] color overlay_widget::background_color() const noexcept
{
    return theme().color(theme_color::fill, semantic_layer + 1);
}

[[nodiscard]] color overlay_widget::foreground_color() const noexcept
{
    return theme().color(theme_color::border, semantic_layer + 1);
}

void overlay_widget::scroll_to_show(tt::rectangle rectangle) noexcept
{
    // An overlay is in an absolute position on the window,
    // so do not forward the scroll_to_show message to its parent.
}

void overlay_widget::draw_background(draw_context context) noexcept
{
    context.draw_box_with_border_outside(rectangle(), background_color(), foreground_color());
}

}
