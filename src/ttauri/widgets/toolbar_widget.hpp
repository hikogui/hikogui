// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include <memory>
#include <ranges>

namespace tt {

class toolbar_widget final : public widget {
public:
    using super = widget;

    toolbar_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
    {
        tt_axiom(is_gui_thread());

        if (parent) {
            // The toolbar widget does draw itself.
            draw_layer = parent->draw_layer + 1.0f;

            // The toolbar is a top level widget, which draws its background as the next level.
            semantic_layer = 0;
        }
        _margin = 0.0f;
    }

    ~toolbar_widget() {}

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    widget &add_widget(horizontal_alignment alignment, std::unique_ptr<widget> widget) noexcept
    {
        auto &tmp = super::add_widget(std::move(widget));
        switch (alignment) {
            using enum horizontal_alignment;
        case left: _left_children.push_back(&tmp); break;
        case right: _right_children.push_back(&tmp); break;
        default: tt_no_default();
        }

        return tmp;
    }

    [[nodiscard]] bool constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
    {
        tt_axiom(is_gui_thread());

        if (super::constrain(display_time_point, need_reconstrain)) {
            auto shared_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 100};
            auto shared_height = 0.0f;

            _layout.clear();
            _layout.reserve(std::ssize(_left_children) + 1 + std::ssize(_right_children));

            ssize_t index = 0;
            for (ttlet &child : _left_children) {
                update_constraints_for_child(*child, index++, shared_base_line, shared_height);
            }

            // Add a space between the left and right widgets.
            _layout.update(index++, theme::global().large_size, theme::global().large_size, 32767.0f, 0.0f);

            for (ttlet &child : std::views::reverse(_right_children)) {
                update_constraints_for_child(*child, index++, shared_base_line, shared_height);
            }

            tt_axiom(index == std::ssize(_left_children) + 1 + std::ssize(_right_children));
            _minimum_size = {_layout.minimum_size(), shared_height};
            _preferred_size = {_layout.preferred_size(), shared_height};
            _maximum_size = {_layout.maximum_size(), shared_height};
            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    void layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_axiom(is_gui_thread());

        need_layout |= _request_layout.exchange(false);
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
        super::layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_axiom(is_gui_thread());

        if (overlaps(context, _clipping_rectangle)) {
            context.draw_filled_quad(rectangle(), theme::global(theme_color::fill, semantic_layer + 1));
        }

        super::draw(std::move(context), display_time_point);
    }

    hitbox hitbox_test(point2 position) const noexcept
    {
        tt_axiom(is_gui_thread());

        auto r = hitbox{};

        if (_visible_rectangle.contains(position)) {
            r = hitbox{this, draw_layer, hitbox::Type::MoveArea};
        }

        for (ttlet &child : _children) {
            r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
        }
        return r;
    }

    /** Add a widget directly to this widget.
     */
    template<typename T, horizontal_alignment Alignment = horizontal_alignment::left, typename... Args>
    T &make_widget(Args &&...args)
    {
        auto widget = std::make_unique<T>(window, this, std::forward<Args>(args)...);
        widget->init();
        return static_cast<T &>(add_widget(Alignment, std::move(widget)));
    }

private:
    std::vector<widget *> _left_children;
    std::vector<widget *> _right_children;
    flow_layout _layout;

    void update_constraints_for_child(
        widget const &child,
        ssize_t index,
        relative_base_line &shared_base_line,
        float &shared_height) noexcept
    {
        tt_axiom(is_gui_thread());

        _layout.update(
            index, child.minimum_size().width(), child.preferred_size().width(), child.maximum_size().width(), child.margin());

        shared_height = std::max(shared_height, child.preferred_size().height() + child.margin() * 2.0f);
    }

    void update_layout_for_child(widget &child, ssize_t index) const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet[child_x, child_width] = _layout.get_offset_and_size(index++);

        ttlet child_rectangle = aarectangle{
            rectangle().left() + child_x,
            rectangle().bottom() + child.margin(),
            child_width,
            rectangle().height() - child.margin() * 2.0f};

        child.set_layout_parameters_from_parent(child_rectangle);
    }
};

} // namespace tt
