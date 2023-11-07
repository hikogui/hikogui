// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/overlay_widget.hpp Defines overlay_widget.
 * @ingroup widgets
 */

module;
#include "../macros.hpp"

#include <coroutine>

export module hikogui_widgets_overlay_widget;
import hikogui_coroutine;
import hikogui_widgets_widget;

export namespace hi { inline namespace v1 {

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

    ~overlay_widget() {}

    /** Constructs an empty overlay widget.
     *
     * @param parent The parent widget.
     */
    overlay_widget(not_null<widget_intf const *> parent) noexcept : super(parent)
    {
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
    Widget& emplace(Args&&...args) noexcept
    {
        hi_axiom(loop::main().on_thread());
        hi_assert(_content == nullptr);

        auto tmp = std::make_unique<Widget>(this, std::forward<Args>(args)...);
        auto& ref = *tmp;
        set_widget(std::move(tmp));
        return ref;
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf &> children(bool include_invisible) noexcept override
    {
        co_yield *_content;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};
        _content_constraints = _content->update_constraints();
        return _content_constraints;
    }
    void set_layout(widget_layout const& context) noexcept override
    {
        _layout = context;

        // The clipping rectangle of the overlay matches the rectangle exactly, with a border around it.
        _layout.clipping_rectangle = context.rectangle() + theme().border_width();

        hilet content_rectangle = context.rectangle();
        _content_shape = box_shape{_content_constraints, content_rectangle, theme().baseline_adjustment()};

        // The content should not draw in the border of the overlay, so give a tight clipping rectangle.
        _content->set_layout(_layout.transform(_content_shape, context.rectangle()));
    }
    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            if (overlaps(context, layout())) {
                draw_background(context);
            }
            _content->draw(context);
        }
    }
    [[nodiscard]] color background_color() const noexcept override
    {
        return theme().color(semantic_color::fill, _layout.layer + 1);
    }
    [[nodiscard]] color foreground_color() const noexcept override
    {
        return theme().color(semantic_color::border, _layout.layer + 1);
    }
    void scroll_to_show(hi::aarectangle rectangle) noexcept override
    {
        // An overlay is in an absolute position on the window,
        // so do not forward the scroll_to_show message to its parent.
    }
    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
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

    void draw_background(draw_context const& context) noexcept
    {
        context.draw_box(
            layout(), layout().rectangle(), background_color(), foreground_color(), theme().border_width(), border_side::outside);
    }
};

}} // namespace hi::v1
