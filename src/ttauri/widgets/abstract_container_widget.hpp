// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"

namespace tt {

class abstract_container_widget : public widget {
public:
    using super = widget;

    abstract_container_widget(gui_window &window, std::shared_ptr<widget> parent) noexcept : super(window, parent)
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

        tt_assume(widget->parent.lock().get() == this);
        _children.push_back(widget);
        _request_reconstrain = true;
        window.requestLayout = true;
        return widget;
    }

    /** Add a widget directly to this widget.
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> make_widget(Args &&... args)
    {
        auto tmp = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        tmp->init();
        return std::static_pointer_cast<T>(add_widget(std::move(tmp)));
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        auto has_constrainted = super::update_constraints(display_time_point, need_reconstrain);

        for (auto &&child : _children) {
            tt_assume(child->parent.lock() == shared_from_this());
            has_constrainted |= child->update_constraints(display_time_point, need_reconstrain);
        }

        return has_constrainted;
    }

    void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        for (auto &&child : _children) {
            tt_assume(child->parent.lock().get() == this);
            child->update_layout(display_time_point, need_layout);
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        for (auto &child : _children) {
            tt_assume(child->parent.lock().get() == this);
            child->draw(child->make_draw_context(context), display_time_point);
        }

        super::draw(std::move(context), display_time_point);
    }

    bool handle_command_recursive(command command, std::vector<std::shared_ptr<widget>> const &reject_list) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        auto handled = false;
        for (auto &child : _children) {
            tt_assume(child->parent.lock().get() == this);
            handled |= child->handle_command_recursive(command, reject_list);
        }
        handled |= super::handle_command_recursive(command, reject_list);
        return handled;
    }

    HitBox hitbox_test(vec window_position) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto r = HitBox{};
        for (ttlet &child : _children) {
            tt_assume(child->parent.lock().get() == this);
            r = std::max(r, child->hitbox_test(window_position));
        }
        return r;
    }

    std::shared_ptr<widget> next_keyboard_widget(
        std::shared_ptr<widget> const &current_keyboard_widget,
        bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        // If current_keyboard_widget is empty, then we need to find the first widget that accepts focus.
        auto found = !current_keyboard_widget;

        // The container widget itself accepts focus.
        if (found && !reverse && accepts_focus()) {
            return std::const_pointer_cast<widget>(shared_from_this());
        }

        ssize_t first = reverse ? ssize(_children) - 1 : 0;
        ssize_t last = reverse ? -1 : ssize(_children);
        ssize_t step = reverse ? -1 : 1;
        for (ssize_t i = first; i != last; i += step) {
            auto &&child = _children[i];

            if (found) {
                // Find the first focus accepting widget.
                if (auto tmp = child->next_keyboard_widget({}, reverse)) {
                    return tmp;
                }

            } else {
                auto tmp = child->next_keyboard_widget(current_keyboard_widget, reverse);
                if (tmp == current_keyboard_widget) {
                    // The current widget was found, but no next widget available in the child.
                    found = true;

                } else if (tmp) {
                    return tmp;
                }
            }
        }

        // The container widget itself accepts focus.
        if (found && reverse && accepts_focus()) {
            return std::const_pointer_cast<widget>(shared_from_this());
        }

        return found ? current_keyboard_widget : std::shared_ptr<widget>{};
    }

protected:
    std::vector<std::shared_ptr<widget>> _children;
};

} // namespace tt
