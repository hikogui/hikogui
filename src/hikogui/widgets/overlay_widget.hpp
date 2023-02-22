// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/overlay_widget.hpp Defines overlay_widget.
 * @ingroup widgets
 */

#pragma once

#include "../GUI/module.hpp"

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
template<fixed_string Name = "">
class overlay_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name ^ "overlay";

    ~overlay_widget() {}

    /** Constructs an empty overlay widget.
     *
     * @param parent The parent widget.
     */
    overlay_widget(widget *parent) noexcept : super(parent)
    {
        if (parent) {
            // The overlay-widget will reset the semantic_layer as it is the bottom
            // layer of this virtual-window. However the draw-layer should be above
            // any other widget drawn.
            semantic_layer = 0;
        }
    }

    void set_widget(std::unique_ptr<widget> new_widget) noexcept
    {
        _content = std::move(new_widget);
        ++global_counter<"overlay_widget:set_widget:constrain">;
        process_event({gui_event_type::window_reconstrain});
    }

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
    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_content;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _content_constraints = _content->update_constraints();
        return _content_constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        layout = context;

        // The clipping rectangle of the overlay matches the rectangle exactly, with a border around it.
        layout.clipping_rectangle = context.rectangle() + theme<prefix ^ "outline.width", int>{}(this);

        hilet content_rectangle = context.rectangle();
        _content_shape = box_shape{_content_constraints, content_rectangle, theme<prefix ^ "cap-height", int>{}(this)};

        // The content should not draw in the border of the overlay, so give a tight clipping rectangle.
        _content->set_layout(layout.transform(_content_shape, 1.0f, context.rectangle()));
    }

    void draw(widget_draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            if (overlaps(context, layout)) {
                draw_background(context);
            }
            _content->draw(context);
        }
    }

    void scroll_to_show(hi::aarectanglei rectangle) noexcept override
    {
        // An overlay is in an absolute position on the window,
        // so do not forward the scroll_to_show message to its parent.
    }

    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial) {
            return _content->hitbox_test_from_parent(position);
        } else {
            return {};
        }
    }
    /// @endprivatesection
private:
    std::unique_ptr<widget> _content;
    box_constraints _content_constraints;
    box_shape _content_shape;

    void draw_background(widget_draw_context const& context) noexcept
    {
        context.draw_box(
            layout,
            layout.rectangle(),
            theme<prefix ^ "fill.color", color>{}(this),
            theme<prefix ^ "outline.color", color>{}(this),
            theme<prefix ^ "outline.width", int>{}(this),
            border_side::outside);
    }
};

}} // namespace hi::v1
