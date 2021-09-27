// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../flow_layout.hpp"
#include "../alignment.hpp"
#include <memory>
#include <ranges>

namespace tt {

/** A toolbar widget is located at the top of a window and lays out its children
 * horizontally.
 *
 * The toolbar widget is shown at the top of a window, it is drawn with a
 * different background than the window's content to differentiate from it.
 *
 * Child widgets may be added on both the left side and right side of the
 * toolbar, a small space in the center is added for visual separation.
 *
 * Child widgets are drawn to full height of the toolbar determined by the
 * preferred size of all contained widgets. The width of each widget is
 * determined by their preferred size
 */
class toolbar_widget final : public widget {
public:
    using super = widget;

    /** Constructs an empty row/column widget.
     *
     * @param window The window.
     * @param parent The parent widget.
     * @param delegate An optional delegate can be used to populate the row/column widget
     *                 during initialization.
     */
    toolbar_widget(gui_window &window, widget *parent) noexcept;

    /** Add a widget directly to this toolbar-widget.
     *
     * When `Alignment` is `horizontal_alignment::left` the new widget is added
     * to the right of the previous added widget on the left side of the
     * toolbar.
     *
     * When `Alignment` is `horizontal_alignment::right` the new widget is added
     * to the left of the previous added widget on the right side of the
     * toolbar.
     *
     * @tparam Widget The type of the widget to be constructed.
     * @tparam Alignment Add widgets to the left, or right.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, horizontal_alignment Alignment = horizontal_alignment::left, typename... Args>
    Widget &make_widget(Args &&...args)
    {
        auto widget = std::make_unique<Widget>(window, this, std::forward<Args>(args)...);
        widget->init();
        return static_cast<Widget &>(add_widget(Alignment, std::move(widget)));
    }

    /// @privatesection
    [[nodiscard]] float margin() const noexcept override;
    [[nodiscard]] bool constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept;
    void layout(utc_nanoseconds display_time_point, bool need_layout) noexcept override;
    void draw(draw_context context, utc_nanoseconds display_time_point) noexcept override;
    hitbox hitbox_test(point2 position) const noexcept override;
    /// @endprivatesection
private:
    std::vector<widget *> _left_children;
    std::vector<widget *> _right_children;
    flow_layout _layout;

    void update_constraints_for_child(widget const &child, ssize_t index, float &shared_height) noexcept;

    void update_layout_for_child(widget &child, ssize_t index) const noexcept;

    /** Add a widget directly to this widget.
     */
    widget &add_widget(horizontal_alignment alignment, std::unique_ptr<widget> widget) noexcept;
};

} // namespace tt
