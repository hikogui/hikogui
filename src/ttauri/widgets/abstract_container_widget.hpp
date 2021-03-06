// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"

namespace tt {

class abstract_container_widget : public widget {
public:
    using super = widget;

    abstract_container_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent) noexcept :
        super(window, parent)
    {
        if (parent) {
            // Most containers will not draw itself, only its children.
            ttlet lock = std::scoped_lock(gui_system_mutex);
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;
    }

    ~abstract_container_widget() {}

    /** Remove and deallocate all child widgets.
     */
    void clear() noexcept
    {
        _children.clear();
        _request_reconstrain = true;
    }

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    std::shared_ptr<widget> add_widget(std::shared_ptr<widget> widget) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        tt_axiom(&widget->parent() == this);
        _children.push_back(widget);
        _request_reconstrain = true;
        window.requestLayout = true;
        return widget;
    }

    [[nodiscard]] widget &front() noexcept
    {
        return *_children.front();
    }

    [[nodiscard]] widget const &front() const noexcept
    {
        return *_children.front();
    }

    [[nodiscard]] widget &back() noexcept
    {
        return *_children.back();
    }

    [[nodiscard]] widget const &back() const noexcept
    {
        return *_children.back();
    }

    [[nodiscard]] std::shared_ptr<abstract_container_widget const> shared_from_this() const noexcept
    {
        return std::static_pointer_cast<abstract_container_widget const>(super::shared_from_this());
    }

    [[nodiscard]] std::shared_ptr<abstract_container_widget> shared_from_this() noexcept
    {
        return std::static_pointer_cast<abstract_container_widget>(super::shared_from_this());
    }

    /** Add a widget directly to this widget.
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> make_widget(Args &&...args)
    {
        auto tmp = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        tmp->init();
        return std::static_pointer_cast<T>(add_widget(std::move(tmp)));
    }

    [[nodiscard]] virtual bool is_toolbar() const noexcept
    {
        return parent().is_toolbar();
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto has_constrainted = super::update_constraints(display_time_point, need_reconstrain);

        for (auto &&child : _children) {
            tt_axiom(child);
            tt_axiom(&child->parent() == this);
            has_constrainted |= child->update_constraints(display_time_point, need_reconstrain);
        }

        return has_constrainted;
    }

    void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        for (auto &&child : _children) {
            tt_axiom(child);
            tt_axiom(&child->parent() == this);
            child->update_layout(display_time_point, need_layout);
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        for (auto &child : _children) {
            tt_axiom(child);
            tt_axiom(&child->parent() == this);

            auto child_context =
                context.make_child_context(child->parent_to_local(), child->local_to_window(), child->clipping_rectangle());
            child->draw(child_context, display_time_point);
        }

        super::draw(std::move(context), display_time_point);
    }

    bool handle_command_recursive(command command, std::vector<std::shared_ptr<widget>> const &reject_list) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto handled = false;
        for (auto &child : _children) {
            tt_axiom(child);
            tt_axiom(&child->parent() == this);
            handled |= child->handle_command_recursive(command, reject_list);
        }
        handled |= super::handle_command_recursive(command, reject_list);
        return handled;
    }

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto r = hit_box{};
        for (ttlet &child : _children) {
            tt_axiom(child);
            tt_axiom(&child->parent() == this);
            r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
        }
        return r;
    }

    std::shared_ptr<widget const> find_first_widget(keyboard_focus_group group) const noexcept
    {
        for (ttlet child : _children) {
            if (child->accepts_keyboard_focus(group)) {
                return child;
            }
        }
        return {};
    }

    std::shared_ptr<widget const> find_last_widget(keyboard_focus_group group) const noexcept
    {
        for (ttlet child : std::views::reverse(_children)) {
            if (child->accepts_keyboard_focus(group)) {
                return child;
            }
        }
        return {};
    }

    std::shared_ptr<widget> find_next_widget(
        std::shared_ptr<widget> const &current_keyboard_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        tt_axiom(direction != keyboard_focus_direction::current);

        // If current_keyboard_widget is empty, then we need to find the first widget that accepts focus.
        auto found = !current_keyboard_widget;

        // The container widget itself accepts focus.
        if (found && direction == keyboard_focus_direction::forward && accepts_keyboard_focus(group)) {
            return std::const_pointer_cast<widget>(super::shared_from_this());
        }

        ssize_t first = direction == keyboard_focus_direction::forward ? 0 : ssize(_children) - 1;
        ssize_t last = direction == keyboard_focus_direction::forward ? ssize(_children) : -1;
        ssize_t step = direction == keyboard_focus_direction::forward ? 1 : -1;
        for (ssize_t i = first; i != last; i += step) {
            auto &&child = _children[i];
            tt_axiom(child);

            if (found) {
                // Find the first focus accepting widget.
                if (auto tmp = child->find_next_widget({}, group, direction)) {
                    return tmp;
                }

            } else {
                auto tmp = child->find_next_widget(current_keyboard_widget, group, direction);
                if (tmp == current_keyboard_widget) {
                    // The current widget was found, but no next widget available in the child.
                    found = true;

                } else if (tmp) {
                    return tmp;
                }
            }
        }

        // The container widget itself accepts focus.
        if (found && direction == keyboard_focus_direction::backward && accepts_keyboard_focus(group)) {
            return std::const_pointer_cast<widget>(super::shared_from_this());
        }

        return found ? current_keyboard_widget : std::shared_ptr<widget>{};
    }

protected:
    std::vector<std::shared_ptr<widget>> _children;
};

} // namespace tt
