// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../label.hpp"
#include "../weak_or_unique_ptr.hpp"

namespace hi::inline v1 {
class toolbar_widget;
class system_menu_widget;
class grid_widget;
class grid_delegate;

class window_widget final : public widget {
public:
    using super = widget;
    using delegate_type = grid_delegate;

    observable<label> title;

    template<typename Title>
    window_widget(gui_window &window, Title &&title, std::weak_ptr<delegate_type> delegate = {}) noexcept :
        super(window, nullptr), title(std::forward<Title>(title)), _content_delegate(std::move(delegate))
    {
        constructor_implementation();
    }

    /** The background color of the window.
     * This function is used during rendering to use the optimized
     * GPU clear function.
     */
    [[nodiscard]] color background_color() noexcept;

    /** Get a reference to the window's content widget.
     * @see grid_widget
     * @return A reference to a grid_widget.
     */
    [[nodiscard]] grid_widget &content() noexcept;

    /** Get a reference to window's toolbar widget.
     * @see toolbar_widget
     * @return A reference to a toolbar_widget.
     */
    [[nodiscard]] toolbar_widget &toolbar() noexcept;

    /** Defining on which edges the resize handle has priority over widget at a higher layer.
     */
    void set_resize_border_priority(bool left, bool right, bool bottom, bool top) noexcept;

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override;
    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &layout) noexcept;
    void draw(draw_context const &context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    /// @endprivatesection
private:
    decltype(title)::token_type _title_cbt;

    std::weak_ptr<delegate_type> _content_delegate;

    aarectangle _content_rectangle;
    std::unique_ptr<grid_widget> _content;

    aarectangle _toolbar_rectangle;
    std::unique_ptr<toolbar_widget> _toolbar;
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
    system_menu_widget *_system_menu = nullptr;
#endif

    bool _left_resize_border_has_priority = true;
    bool _right_resize_border_has_priority = true;
    bool _bottom_resize_border_has_priority = true;
    bool _top_resize_border_has_priority = true;

    void constructor_implementation() noexcept;
};

} // namespace hi::inline v1
