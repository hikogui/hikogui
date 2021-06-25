// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../label.hpp"
#include "../weak_or_unique_ptr.hpp"

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

    template<typename Title>
    window_widget(gui_window &window, Title &&title, weak_or_unique_ptr<gui_window_delegate> delegate) noexcept :
        super(window, nullptr), _content_delegate(std::move(delegate))
    {
        title = std::forward<Title>(title);
    }

    template<typename Title>
    window_widget(gui_window &window, Title &&title) noexcept :
        window_widget(
            window,
            std::forward<Title>(title),
            std::make_unique<grid_layout_delegate>())
    {
    }

    void init() noexcept override;

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;
    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override;

    [[nodiscard]] color backgroundColor() noexcept
    {
        tt_axiom(is_gui_thread());
        return theme::global(theme_color::fill, _semantic_layer);
    }

    /** Defining on which edges the resize handle has priority over widget at a higher layer.
     */
    void set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept
    {
        tt_axiom(is_gui_thread());
        _left_resize_border_has_priority = left;
        _right_resize_border_has_priority = right;
        _bottom_resize_border_has_priority = bottom;
        _top_resize_border_has_priority = top;
    }

    [[nodiscard]] grid_layout_widget &content() noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(_content);
        return *_content;
    }

    [[nodiscard]] toolbar_widget &toolbar() noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(_toolbar);
        return *_toolbar;
    }

private:
    decltype(title)::callback_ptr_type _title_callback;

    weak_or_unique_ptr<grid_layout_delegate> _content_delegate;
    grid_layout_widget *_content = nullptr;
    toolbar_widget *_toolbar = nullptr;
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    system_menu_widget *_system_menu = nullptr;
#endif

    bool _left_resize_border_has_priority = true;
    bool _right_resize_border_has_priority = true;
    bool _bottom_resize_border_has_priority = true;
    bool _top_resize_border_has_priority = true;
};

} // namespace tt
