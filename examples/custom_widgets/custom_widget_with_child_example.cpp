// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/widget.hpp"
#include "ttauri/widgets/label_widget.hpp"
#include "ttauri/crt.hpp"

// Every widget must inherit from tt::widget.
class widget_with_child : public tt::widget {
public:
    // Every constructor of a widget starts with a `window` and `parent` argument.
    // In most cases these are automatically filled in when calling a container widget's `make_widget()` function.
    template<typename Label>
    widget_with_child(tt::gui_window &window, tt::widget *parent, Label &&label) noexcept : widget(window, parent)
    {
        // Our child widget is a `label_widget` which requires a label to be passed as an third argument.
        // We use a templated argument to forward the label into the `label_widget`.
        _label_widget = std::make_unique<tt::label_widget>(window, this, std::forward<Label>(label));
    }

    // The set_constraints() function is called when the window is first initialized,
    // or when a widget wants to change its constraints.
    tt::widget_constraints const &set_constraints() noexcept override
    {
        // Almost all widgets will reset the `_layout` variable here so that it will
        // trigger the calculations in `set_layout()` as well.
        _layout = {};

        // We need to recursively set the constraints of any child widget here as well
        auto const label_constraints = _label_widget->set_constraints();

        // We add the ability to resize the widget beyond the size of the label.
        _constraints.minimum = label_constraints.minimum;
        _constraints.preferred = label_constraints.preferred + theme().margin;
        _constraints.maximum = label_constraints.maximum + tt::extent2{100.0f, 50.0f};
        _constraints.margin = theme().margin;
        return _constraints;
    }

    // The `set_layout()` function is called when the window has resized, or when
    // a widget wants to change the internal layout.
    //
    // NOTE: The size of the layout may be larger than the maximum constraints of this widget.
    void set_layout(tt::widget_layout const &context) noexcept override
    {
        // Update the `_layout` with the new context, in this case we want to do some
        // calculations when the size of the widget was changed.
        if (_layout.store(context) >= tt::layout_update::size) {
            // The layout of the child widget are also calculated here, which only needs to be done
            // when the layout of the current widget changes.
            _label_rectangle = align(_layout.rectangle(), _label_widget->constraints().preferred, tt::alignment::middle_center);
        }

        // The layout of any child widget must always be set, even if the layout didn't actually change.
        // This is because child widgets may need to re-layout for other reasons.
        _label_widget->set_layout(_label_rectangle * context);
    }

    // The `draw()` function is called when all or part of the window requires redrawing.
    // This may happen when showing the window for the first time, when the operating-system
    // requests a (partial) redraw, or when a widget requests a redraw of itself.
    void draw(tt::draw_context const &context) noexcept override
    {
        if (visible) {
            // We only need to draw the widget when it is visible and when the visible area of
            // the widget overlaps with the scissor-rectangle (partial redraw) of the drawing context.
            if (overlaps(context, layout())) {
                // There may be stylistic reasons to draw into the margin, for example
                // round objects need to be drawn slightly larger than square objects.
                // The standard clipping rectangle is 2 pixels larger than the _layout.rectangle().
                // In this example we draw the border outside the rectangle.
                context.draw_box(
                    _layout,
                    _layout.rectangle(),
                    background_color(),
                    foreground_color(),
                    theme().border_width,
                    tt::border_side::outside,
                    theme().rounding_radius);
            }

            // Child widget only need to be drawn when the parent is visible, but the child may have
            // a visible area outside of the parent's visible area, therefor it should do its own
            // overlap check.
            _label_widget->draw(context);
        }
    }

protected:
    // This function MUST be overridden when a widget has children.
    //
    // The order of the children returned is used for determining the next widget for
    // keyboard navigation.
    //
    // The allocator argument should not be used by the function, it is used by the caller
    // to allocate the co-routine's frame on the stack.
    [[nodiscard]] tt::pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        // This function is often written as a co-routine that yields a pointer to each of its children.
        co_yield _label_widget.get();
    }

private:
    // Child widgets are owned by their parent.
    std::unique_ptr<tt::label_widget> _label_widget;
    tt::aarectangle _label_rectangle;
};

int tt_main(int argc, char *argv[])
{
    auto gui = tt::gui_system::make_unique();
    auto &window = gui->make_window(tt::l10n("Widget with child"));
    window.content().make_widget<widget_with_child>("A1", tt::l10n("Widget with child"));
    return gui->loop();
}