// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "overlay_widget.hpp"
#include "../GUI/theme.hpp"

namespace tt {
inline namespace v1 {

overlay_widget::overlay_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    super(window, parent), _delegate(std::move(delegate))
{
    if (parent) {
        // The overlay-widget will reset the semantic_layer as it is the bottom
        // layer of this virtual-window. However the draw-layer should be above
        // any other widget drawn.
        semantic_layer = 0;
    }

    if (auto d = _delegate.lock()) {
        d->init(*this);
    }
}

overlay_widget::~overlay_widget()
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

void overlay_widget::set_widget(std::unique_ptr<widget> new_widget) noexcept
{
    _content = std::move(new_widget);
    window.request_reconstrain();
}

widget_constraints const &overlay_widget::set_constraints() noexcept
{
    _layout = {};
    return _constraints = _content->set_constraints();
}

void overlay_widget::set_layout(widget_layout const &context_) noexcept
{
    if (visible) {
        // The clipping rectangle of the overlay matches the rectangle exactly, with a border around it.
        ttlet context = context_.override_clip(context_.rectangle() + theme().border_width);
        _layout.store(context);

        _content->set_layout(layout().rectangle() * context);
    }
}

void overlay_widget::draw(draw_context const &context) noexcept
{
    if (visible) {
        if (overlaps(context, layout())) {
            draw_background(context);
        }
        _content->draw(context);
    }
}

[[nodiscard]] color overlay_widget::background_color() const noexcept
{
    return theme().color(theme_color::fill, semantic_layer + 1);
}

[[nodiscard]] color overlay_widget::foreground_color() const noexcept
{
    return theme().color(theme_color::border, semantic_layer + 1);
}

void overlay_widget::scroll_to_show(tt::aarectangle rectangle) noexcept
{
    // An overlay is in an absolute position on the window,
    // so do not forward the scroll_to_show message to its parent.
}

void overlay_widget::draw_background(draw_context const &context) noexcept
{
    context.draw_box(
        layout(), layout().rectangle(), background_color(), foreground_color(), theme().border_width, border_side::outside);
}

}
} // namespace tt
