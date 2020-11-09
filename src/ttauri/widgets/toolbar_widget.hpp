// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_container_widget.hpp"
#include <memory>

namespace tt {

class toolbar_widget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    toolbar_widget(Window &window, std::shared_ptr<widget> parent) noexcept : super(window, parent)
    {
        if (parent) {
            // The toolbar widget does draw itself.
            ttlet lock = std::scoped_lock(GUISystem_mutex);
            _draw_layer = parent->draw_layer() + 1.0f;

            // The toolbar is a top level widget, which draws its background as the next level.
            _semantic_layer = 0;
        }
    }

    ~toolbar_widget() {}

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    std::shared_ptr<widget> add_widget(horizontal_alignment alignment, std::shared_ptr<widget> widget) noexcept
    {
        auto tmp = super::add_widget(std::move(widget));
        switch (alignment) {
            using enum horizontal_alignment;
        case left: _left_children.push_back(tmp); break;
        case right: _right_children.push_back(tmp); break;
        default: tt_no_default();
        }

        return tmp;
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            auto shared_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 100};
            auto shared_height = finterval{};

            _layout.clear();
            _layout.reserve(std::ssize(_left_children) + 1 + std::ssize(_right_children));

            ssize_t index = 0;
            for (ttlet &child : _left_children) {
                update_constraints_for_child(*child, index++, shared_base_line, shared_height);
            }

            // Add a space between the left and right widgets.
            _layout.update(
                index++,
                finterval{Theme::width, std::numeric_limits<float>::max()},
                ranged_int<3>{1},
                0.0f,
                relative_base_line{});

            for (ttlet &child : std::views::reverse(_right_children)) {
                update_constraints_for_child(*child, index++, shared_base_line, shared_height);
            }

            tt_assume(index == std::ssize(_left_children) + 1 + std::ssize(_right_children));
            _preferred_size = {_layout.extent(), finterval{shared_height.minimum()}};
            _preferred_base_line = shared_base_line;
            return true;
        } else {
            return false;
        }
    }

    bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _layout.update_layout(rectangle().width());

            ssize_t index = 0;
            for (ttlet &child : _left_children) {
                update_layout_for_child(*child, index++);
            }

            // Skip over the cell between left and right children.
            index++;

            for (ttlet &child : std::views::reverse(_right_children)) {
                update_layout_for_child(*child, index++);
            }

            tt_assume(index == std::ssize(_left_children) + 1 + std::ssize(_right_children));
        }
        return abstract_container_widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        context.fillColor = theme->fillColor(_semantic_layer + 1);
        context.drawFilledQuad(rectangle());
        abstract_container_widget::draw(std::move(context), display_time_point);
    }

    HitBox hitbox_test(vec window_position) const noexcept
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);

        auto r = HitBox{};

        if (_window_clipping_rectangle.contains(window_position)) {
            r = HitBox{weak_from_this(), _draw_layer, HitBox::Type::MoveArea};
        }

        for (ttlet &child : _children) {
            r = std::max(r, child->hitbox_test(window_position));
        }
        return r;
    }

    /** Add a widget directly to this widget.
     */
    template<typename T, horizontal_alignment Alignment = horizontal_alignment::left, typename... Args>
    std::shared_ptr<T> make_widget(Args &&... args)
    {
        auto widget = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        widget->initialize();
        return std::static_pointer_cast<T>(add_widget(Alignment, std::move(widget)));
    }

private:
    std::vector<std::shared_ptr<widget>> _left_children;
    std::vector<std::shared_ptr<widget>> _right_children;
    flow_layout _layout;

    void update_constraints_for_child(
        widget const &child,
        ssize_t index,
        relative_base_line &shared_base_line,
        finterval &shared_height) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        _layout.update(index, child.preferred_size().width(), child.width_resistance(), child.margin(), relative_base_line{});

        shared_base_line = std::max(shared_base_line, child.preferred_base_line());
        shared_height = intersect(shared_height, child.preferred_size().height() + child.margin() * 2.0f);
    }

    void update_layout_for_child(widget &child, ssize_t index) const noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        ttlet[child_x, child_width] = _layout.get_offset_and_size(index++);

        ttlet child_rectangle = aarect{
            rectangle().x() + child_x,
            rectangle().y() + child.margin(),
            child_width,
            rectangle().height() - child.margin() * 2.0f};

        ttlet child_window_rectangle = mat::T2{_window_rectangle} * child_rectangle;

        child.set_layout_parameters(child_window_rectangle, _window_clipping_rectangle, _window_base_line);
    }
};

} // namespace tt
