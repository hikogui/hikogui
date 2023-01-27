// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toolbar_widget.hpp Defines toolbar_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../layout/row_column_layout.hpp"
#include "../geometry/module.hpp"
#include <memory>
#include <ranges>

namespace hi { inline namespace v1 {

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
 *
 * @ingroup widgets
 */
class toolbar_widget final : public widget {
public:
    using super = widget;

    /** Constructs an empty row/column widget.
     *
     * @param parent The parent widget.
     */
    toolbar_widget(widget *parent) noexcept;

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
    Widget& make_widget(Args&&...args)
    {
        auto widget = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        return static_cast<Widget&>(add_widget(Alignment, std::move(widget)));
    }

    /// @privatesection
    [[nodiscard]] generator<widget const &> children(bool include_invisible) const noexcept override
    {
        for (hilet& child : _children) {
            co_yield *child.value;
        }
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    hitbox hitbox_test(point2i position) const noexcept override;
    [[nodiscard]] color focus_color() const noexcept override;
    /// @endprivatesection
private:
    mutable row_layout<std::unique_ptr<widget>> _children;
    mutable int _child_height_adjustment = 0;
    size_t _spacer_index = 0;

    void update_layout_for_child(widget& child, ssize_t index, widget_layout const& context) const noexcept;

    /** Add a widget directly to this widget.
     */
    widget& add_widget(horizontal_alignment alignment, std::unique_ptr<widget> widget) noexcept;

    /** Check if a child tab-button has focus.
     *
     * @return If true the toolbar should draw a focus bar.
     */
    bool tab_button_has_focus() const noexcept;
};

}} // namespace hi::v1
