// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "overlay_widget.hpp"
#include "../GUI/theme.hpp"

namespace hi::inline v1 {

overlay_widget::overlay_widget(widget *parent) noexcept :
    super(parent)
{
    if (parent) {
        // The overlay-widget will reset the semantic_layer as it is the bottom
        // layer of this virtual-window. However the draw-layer should be above
        // any other widget drawn.
        semantic_layer = 0;
    }
}

overlay_widget::~overlay_widget()
{
}

void overlay_widget::set_widget(std::unique_ptr<widget> new_widget) noexcept
{
    _content = std::move(new_widget);
    ++global_counter<"overlay_widget:set_widget:constrain">;
    process_event({gui_event_type::window_reconstrain});
}

widget_constraints const& overlay_widget::set_constraints(set_constraints_context const& context) noexcept
{
    _layout = {};
    return _constraints = _content->set_constraints(context);
}

void overlay_widget::set_layout(widget_layout const& context) noexcept
{
    _layout = context;

    // The clipping rectangle of the overlay matches the rectangle exactly, with a border around it.
    _layout.clipping_rectangle = context.rectangle() + context.theme->border_width;

    // The content should not draw in the border of the overlay, so give a tight clipping rectangle.
    _content->set_layout(_layout.transform(context.rectangle(), 1.0f, context.rectangle()));
}

void overlay_widget::draw(draw_context const &context) noexcept
{
    if (*mode > widget_mode::invisible) {
        if (overlaps(context, layout())) {
            draw_background(context);
        }
        _content->draw(context);
    }
}

[[nodiscard]] color overlay_widget::background_color() const noexcept
{
    return _layout.theme->color(semantic_color::fill, semantic_layer + 1);
}

[[nodiscard]] color overlay_widget::foreground_color() const noexcept
{
    return _layout.theme->color(semantic_color::border, semantic_layer + 1);
}

void overlay_widget::scroll_to_show(hi::aarectangle rectangle) noexcept
{
    // An overlay is in an absolute position on the window,
    // so do not forward the scroll_to_show message to its parent.
}

void overlay_widget::draw_background(draw_context const &context) noexcept
{
    context.draw_box(
        layout(),
        layout().rectangle(),
        background_color(),
        foreground_color(),
        layout().theme->border_width,
        border_side::outside);
}

[[nodiscard]] hitbox overlay_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial) {
        return _content->hitbox_test_from_parent(position);
    } else {
        return {};
    }
}

} // namespace hi::inline v1
