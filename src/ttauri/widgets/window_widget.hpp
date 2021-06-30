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
class grid_widget;
class grid_delegate;

class window_widget final : public widget {
public:
    using super = widget;
    using delegate_type = grid_delegate;

    observable<label> title;

    ~window_widget();

    template<typename Title>
    window_widget(gui_window &window, Title &&title, std::weak_ptr<delegate_type> delegate = {}) noexcept :
        super(window, nullptr), title(std::forward<Title>(title)), _content_delegate(std::move(delegate))
    {
    }

    void init() noexcept override;

    [[nodiscard]] bool
    constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;
    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override;

    [[nodiscard]] color backgroundColor() noexcept
    {
        tt_axiom(is_gui_thread());
        return theme::global(theme_color::fill, semantic_layer);
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

    [[nodiscard]] grid_widget &content() noexcept
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

    std::weak_ptr<delegate_type> _content_delegate;
    grid_widget *_content = nullptr;
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
