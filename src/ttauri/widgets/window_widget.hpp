// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../label.hpp"

namespace tt {

class toolbar_widget;
class system_menu_widget;
class grid_layout_widget;
class grid_layout_delegate;

class window_widget final : public widget {
public:
    using super = widget;

    observable<label> title;

    ~window_widget();

    window_widget(gui_window &window, std::shared_ptr<grid_layout_delegate> delegate) noexcept;

    template<typename Title>
    window_widget(gui_window &window, std::shared_ptr<grid_layout_delegate> delegate, Title &&title) noexcept :
        window_widget(window, std::move(delegate))
    {
        this->title = std::forward<Title>(title);
    }

    void init() noexcept override;

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;
    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept override;

    [[nodiscard]] color backgroundColor() noexcept {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return theme::global->fill_color(_semantic_layer);
    }

    /** Defining on which edges the resize handle has priority over widget at a higher layer.
     */
    void set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        _left_resize_border_has_priority = left;
        _right_resize_border_has_priority = right;
        _bottom_resize_border_has_priority = bottom;
        _top_resize_border_has_priority = top;
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

private:
    decltype(title)::callback_ptr_type _title_callback;

    std::shared_ptr<grid_layout_delegate> _content_delegate;
    std::shared_ptr<grid_layout_widget> _content;
    std::shared_ptr<toolbar_widget> _toolbar;
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    std::shared_ptr<system_menu_widget> _system_menu;
#endif

    bool _left_resize_border_has_priority = true;
    bool _right_resize_border_has_priority = true;
    bool _bottom_resize_border_has_priority = true;
    bool _top_resize_border_has_priority = true;
};

} // namespace tt
