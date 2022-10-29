// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/overlay_widget.hpp Defines overlay_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"

namespace hi { inline namespace v1 {

/** A GUI widget which may exist anywhere on a window overlaid above any other widget.
 * @ingroup widgets
 *
 * The overlay widget allows a content widget to be shown on top of other
 * widgets in the window. It may be used for pop-up widgets, dialog boxes and
 * sheets.
 *
 * The size of the overlay widget is based on the `widget::minimum_size()`,
 * `widget::preferred_size()` and `widget::maximum_size()`. Unlike other
 * container widgets the clipping rectangle is made tightly around the container
 * widget so that no drawing will happen outside of the overlay. The overlay
 * itself will draw outside the clipping rectangle, for drawing a border and
 * potentially a shadow.
 *
 * As an overlay widget still confined to a window, like other widgets, when
 * setting its layout parameters, it is recommended to use
 * `widget::make_overlay_rectangle()` to make a rectangle that will fit inside
 * the window.
 *
 * It is recommended that the content of an overlay widget is a scroll widget
 * so that when the overlay widget is drawn smaller than the requested rectangle
 * the content will behave correctly.
 */
class overlay_widget final : public widget {
public:
    using super = widget;

    ~overlay_widget();

    /** Constructs an empty overlay widget.
     *
     * @param parent The parent widget.
     */
    overlay_widget(widget *parent) noexcept;

    void set_widget(std::unique_ptr<widget> new_widget) noexcept;

    /** Add a content widget directly to this overlay widget.
     *
     * This widget is added as the content widget.
     *
     * @pre No content widgets have been added before.
     * @tparam Widget The type of the widget to be constructed.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget& make_widget(Args&&...args) noexcept
    {
        hi_axiom(loop::main().on_thread());
        hi_assert(_content == nullptr);

        auto tmp = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        auto& ref = *tmp;
        set_widget(std::move(tmp));
        return ref;
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        co_yield _content.get();
    }

    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] color background_color() const noexcept override;
    [[nodiscard]] color foreground_color() const noexcept override;
    void scroll_to_show(hi::aarectangle rectangle) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    /// @endprivatesection
private:
    std::unique_ptr<widget> _content;

    void draw_background(draw_context const& context) noexcept;
};

}} // namespace hi::v1
