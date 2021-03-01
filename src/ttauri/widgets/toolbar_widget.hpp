// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_container_widget.hpp"
#include <memory>

namespace tt {

class toolbar_widget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    toolbar_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent) noexcept : super(window, parent)
    {
        if (parent) {
            // The toolbar widget does draw itself.
            ttlet lock = std::scoped_lock(gui_system_mutex);
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
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            auto shared_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 100};
            auto shared_height = 0.0f;

            _layout.clear();
            _layout.reserve(std::ssize(_left_children) + 1 + std::ssize(_right_children));

            ssize_t index = 0;
            for (ttlet &child : _left_children) {
                update_constraints_for_child(*child, index++, shared_base_line, shared_height);
            }

            // Add a space between the left and right widgets.
            _layout.update(
                index++,
                theme::global->width,
                ranged_int<3>{0},
                0.0f);

            for (ttlet &child : std::views::reverse(_right_children)) {
                update_constraints_for_child(*child, index++, shared_base_line, shared_height);
            }

            tt_axiom(index == std::ssize(_left_children) + 1 + std::ssize(_right_children));
            _preferred_size = {
                extent2{_layout.minimum_size(), shared_height},
                extent2{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _layout.set_size(rectangle().width());

            ssize_t index = 0;
            for (ttlet &child : _left_children) {
                update_layout_for_child(*child, index++);
            }

            // Skip over the cell between left and right children.
            index++;

            for (ttlet &child : std::views::reverse(_right_children)) {
                update_layout_for_child(*child, index++);
            }

            tt_axiom(index == std::ssize(_left_children) + 1 + std::ssize(_right_children));
        }
        abstract_container_widget::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, _clipping_rectangle)) {
            context.draw_filled_quad(rectangle(), theme::global->fillColor(_semantic_layer + 1));
        }

        abstract_container_widget::draw(std::move(context), display_time_point);
    }

    hit_box hitbox_test(point2 position) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto r = hit_box{};

        if (_clipping_rectangle.contains(position)) {
            r = hit_box{weak_from_this(), _draw_layer, hit_box::Type::MoveArea};
        }

        for (ttlet &child : _children) {
            r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
        }
        return r;
    }

    /** Add a widget directly to this widget.
     */
    template<typename T, horizontal_alignment Alignment = horizontal_alignment::left, typename... Args>
    std::shared_ptr<T> make_widget(Args &&... args)
    {
        auto widget = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        widget->init();
        return std::static_pointer_cast<T>(add_widget(Alignment, std::move(widget)));
    }

    [[nodiscard]] bool is_toolbar() const noexcept override
    {
        return true;
    }

private:
    std::vector<std::shared_ptr<widget>> _left_children;
    std::vector<std::shared_ptr<widget>> _right_children;
    flow_layout _layout;

    void update_constraints_for_child(
        widget const &child,
        ssize_t index,
        relative_base_line &shared_base_line,
        float &shared_height) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        _layout.update(index, child.preferred_size().minimum().width(), child.width_resistance(), child.margin());

        shared_height = std::max(shared_height, child.preferred_size().minimum().height() + child.margin() * 2.0f);
    }

    void update_layout_for_child(widget &child, ssize_t index) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet[child_x, child_width] = _layout.get_offset_and_size(index++);

        ttlet child_rectangle = aarect{
            rectangle().x() + child_x,
            rectangle().y() + child.margin(),
            child_width,
            rectangle().height() - child.margin() * 2.0f};

        child.set_layout_parameters_from_parent(child_rectangle);
    }
};

} // namespace tt
