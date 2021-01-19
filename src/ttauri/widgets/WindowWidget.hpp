// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_container_widget.hpp"
#include "../label.hpp"

namespace tt {

class toolbar_widget;
class grid_layout_widget;

class WindowWidget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    WindowWidget(gui_window &window, std::weak_ptr<grid_layout_delegate> const &delegate, label title) noexcept;
    ~WindowWidget();

    void init() noexcept override;

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;
    [[nodiscard]] HitBox hitbox_test(f32x4 window_position) const noexcept override;

    [[nodiscard]] f32x4 backgroundColor() noexcept {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return theme::global->fillColor(_semantic_layer);
    }

    /** Defining on which edges the resize handle has priority over widget at a higher layer.
     */
    void set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        left_resize_border_has_priority = left;
        right_resize_border_has_priority = right;
        bottom_resize_border_has_priority = bottom;
        top_resize_border_has_priority = top;
    }

    [[nodiscard]] std::shared_ptr<grid_layout_widget> content() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(_content);
        return _content;
    }

    [[nodiscard]] std::shared_ptr<toolbar_widget> toolbar() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(_toolbar);
        return _toolbar;
    }

    [[nodiscard]] bool is_toolbar() const noexcept override
    {
        return false;
    }

private:
    label title;
    std::weak_ptr<grid_layout_delegate> _content_delegate;
    std::shared_ptr<grid_layout_widget> _content;
    std::shared_ptr<toolbar_widget> _toolbar;

    bool left_resize_border_has_priority = true;
    bool right_resize_border_has_priority = true;
    bool bottom_resize_border_has_priority = true;
    bool top_resize_border_has_priority = true;
};

} // namespace tt
