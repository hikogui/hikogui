// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"

namespace tt::inline v1 {

class scroll_aperture_widget : public widget {
public:
    using super = widget;

    observable<float> content_width;
    observable<float> content_height;
    observable<float> aperture_width;
    observable<float> aperture_height;
    observable<float> offset_x;
    observable<float> offset_y;

    scroll_aperture_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
    {
        tt_axiom(is_gui_thread());
        tt_axiom(parent);

        // The aperture-widget will not draw itself, only its selected content.
        semantic_layer = parent->semantic_layer;

        content_width.subscribe(_relayout_callback);
        content_height.subscribe(_relayout_callback);
        aperture_width.subscribe(_relayout_callback);
        aperture_height.subscribe(_relayout_callback);
        offset_x.subscribe(_relayout_callback);
        offset_y.subscribe(_relayout_callback);
    }

    template<typename Widget, typename... Args>
    Widget &make_widget(Args &&...args) noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(not _content);

        auto tmp = std::make_unique<Widget>(window, this, std::forward<Args>(args)...);
        auto &ref = *tmp;
        _content = std::move(tmp);
        return ref;
    }

    [[nodiscard]] bool x_axis_scrolls() const noexcept
    {
        return content_width > aperture_width;
    }

    [[nodiscard]] bool y_axis_scrolls() const noexcept
    {
        return content_height > aperture_height;
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        co_yield _content.get();
    }

    widget_constraints const &set_constraints() noexcept override
    {
        _layout = {};

        tt_axiom(_content);
        ttlet content_constraints = _content->set_constraints();

        ttlet minimum_size = extent2{
            content_constraints.margins.left() + content_constraints.minimum.width() + content_constraints.margins.right(),
            content_constraints.margins.top() + content_constraints.minimum.height() + content_constraints.margins.bottom()};
        ttlet preferred_size = extent2{
            content_constraints.margins.left() + content_constraints.preferred.width() + content_constraints.margins.right(),
            content_constraints.margins.top() + content_constraints.preferred.height() + content_constraints.margins.bottom()};
        ttlet maximum_size = extent2{
            content_constraints.margins.left() + content_constraints.maximum.width() + content_constraints.margins.right(),
            content_constraints.margins.top() + content_constraints.maximum.height() + content_constraints.margins.bottom()};

        return _constraints = {minimum_size, preferred_size, maximum_size, margins{}};
    }

    void set_layout(widget_layout const &layout) noexcept override
    {
        ttlet content_constraints = _content->constraints();
        ttlet margins = content_constraints.margins;

        if (compare_store(_layout, layout)) {
            ttlet preferred_size = content_constraints.preferred;

            aperture_width = layout.width() - margins.left() - margins.right();
            aperture_height = layout.height() - margins.bottom() - margins.top();

            ttlet aperture_width_ = *aperture_width.cget();
            ttlet aperture_height_ = *aperture_height.cget();

            // Start scrolling with the preferred size as minimum, so
            // that widgets in the content don't get unnecessarily squeezed.
            content_width = aperture_width_ < preferred_size.width() ? preferred_size.width() : aperture_width_;
            content_height = aperture_height_ < preferred_size.height() ? preferred_size.height() : aperture_height_;
        }

        // Make sure the offsets are limited to the scrollable area.
        ttlet offset_x_max = std::max(content_width - aperture_width, 0.0f);
        ttlet offset_y_max = std::max(content_height - aperture_height, 0.0f);
        offset_x = std::clamp(std::round(*offset_x.cget()), 0.0f, offset_x_max);
        offset_y = std::clamp(std::round(*offset_y.cget()), 0.0f, offset_y_max);

        // The position of the content rectangle relative to the scroll view.
        // The size is further adjusted if the either the horizontal or vertical scroll bar is invisible.
        _content_rectangle = {
            -offset_x + margins.left(), -offset_y + margins.bottom(), *content_width.cget(), *content_height.cget()};

        // The content needs to be at a higher elevation, so that hitbox check
        // will work correctly for handling scrolling with mouse wheel.
        _content->set_layout(layout.transform(_content_rectangle, 1.0f, layout.rectangle()));
    }

    void draw(draw_context const &context) noexcept
    {
        if (visible) {
            _content->draw(context);
        }
    }

    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override
    {
        tt_axiom(is_gui_thread());

        if (visible and enabled) {
            auto r = _content->hitbox_test_from_parent(position);

            if (layout().contains(position)) {
                r = std::max(r, hitbox{this, position});
            }
            return r;

        } else {
            return {};
        }
    }

    bool handle_event(mouse_event const &event) noexcept override
    {
        tt_axiom(is_gui_thread());
        auto handled = super::handle_event(event);

        if (event.type == mouse_event::Type::Wheel) {
            handled = true;
            ttlet new_offset_x = *offset_x.cget() + event.wheelDelta.x() * theme().scale;
            ttlet new_offset_y = *offset_y.cget() + event.wheelDelta.y() * theme().scale;
            ttlet max_offset_x = std::max(0.0f, content_width - aperture_width);
            ttlet max_offset_y = std::max(0.0f, content_height - aperture_height);

            offset_x = std::clamp(new_offset_x, 0.0f, max_offset_x);
            offset_y = std::clamp(new_offset_y, 0.0f, max_offset_y);
            request_relayout();
            return true;
        }
        return handled;
    }

    void scroll_to_show(tt::aarectangle to_show) noexcept override
    {
        auto safe_rectangle = intersect(_layout.rectangle(), _layout.clipping_rectangle);
        float delta_x = 0.0f;
        float delta_y = 0.0f;

        if (safe_rectangle.width() > theme().margin and safe_rectangle.height() > theme().margin) {
            // This will look visually better, if the selected widget is moved with some margin from
            // the edge of the scroll widget. The margins of the content do not have anything to do
            // with the margins that are needed here.
            safe_rectangle = safe_rectangle - theme().margin;

            if (to_show.right() > safe_rectangle.right()) {
                delta_x = to_show.right() - safe_rectangle.right();
            } else if (to_show.left() < safe_rectangle.left()) {
                delta_x = to_show.left() - safe_rectangle.left();
            }

            if (to_show.top() > safe_rectangle.top()) {
                delta_y = to_show.top() - safe_rectangle.top();
            } else if (to_show.bottom() < safe_rectangle.bottom()) {
                delta_y = to_show.bottom() - safe_rectangle.bottom();
            }

            // Scroll the widget
            offset_x += delta_x;
            offset_y += delta_y;
        }

        // There may be recursive scroll view, and they all need to move until the rectangle is visible.
        if (parent) {
            parent->scroll_to_show(bounding_rectangle(_layout.to_parent * translate2(delta_x, delta_y) * to_show));
        }
    }
    /// @endprivatesection
private:
    aarectangle _content_rectangle;
    std::unique_ptr<widget> _content;
};

} // namespace tt::inline v1
