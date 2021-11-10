How to make widgets
===================

Visual
------
A widget is rendered in three steps:
 1. `widget::set_constraints()` is called to determine the minimum, preferred & maximum size and margin of the widget.
 2. `widget::set_layout()` is called to set the size and coordinate-transformations for the widget.
 3. `widget::draw()` is called to (partially) draw the widget.

Each of the three steps is optional, with increasing order of likeliness to be called on each
vertical-sync.

### Constraints
When the window is first opened or when a widget requests a reconstrain; all the widgets are requested to give
their size and other attributes needed for laying-out the widgets.

Currently the constrain attributes are:

 | attribute   | description                                                   |
 |:----------- |:------------------------------------------------------------- |
 | `minimum`   | The absolute minimum size the widget MUST be laid out as.     |
 | `preferred` | The preferred size the widget CAN be laid out as.             |
 | `maximum`   | The maximum size the widget SHOULD be laid out as.            |
 | `margin`    | The minimum margin to other sibling widgets or parent's edge. |

To calculate the constraints of a widget potentially expensive calculations may need to be performed,
such as loading glyph metrics and doing initial text shaping to determine the size of a label.
It is recommended to store these calculations in member variables of your custom widget.
Note: In the example below `_layout` is reset so that calculations that depend on these member variables
are triggered inside `set_layout()`.

Therefor it is recommended to not request a reconstrain, unless the widget is expecting it's constraints
to change, such as when the text in the label changes.

The following is an example of `set_constraints()` function for a widget with a label-child.
You can see that the constraints are based on the constraints of the label combined with sizes
taken from the theme. The label widget itself is based on `theme().size` and the width of the text.

```cpp
tt::widget_constraints const &set_constraints() noexcept override
{
    _layout = {};

    auto const label_constraints = _label_widget->set_constraints();
    _constraints.minimum = label_constraints.minimum;
    _constraints.preferred = label_constraints.preferred + theme().margin;
    _constraints.maximum = label_constraints.maximum + tt::extent2{100.0f, 50.0f};
    _constraints.margin = theme().margin;
    return _constraints;
}
```

### Layout
After constraining or when a widget request a relayout; all widgets are laid out by their parents, based
on the previous information gathered during constraining.

A `widget_layout` currently consists of the following attributes:

 | attribute            | description                                                        |
 |:-------------------- |:------------------------------------------------------------------ |
 | `size`               | The size of the widget.                                            |
 | `to_parent`          | Transformation matrix to convert coordinates from local to parent. |
 | `from_parent`        | Transformation matrix to convert coordinates from parent to local. |
 | `to_window`          | Transformation matrix to convert coordinates from local to window. |
 | `from_window`        | Transformation matrix to convert coordinates from window to local. |
 | `clipping_rectangle` | The axis aligned rectangle to clip any drawing.                    |
 | `display_time_point` | The time when the drawing will appear on the screen.               |

A layout can be transformed to child size & coordinates, by combining the layout with a
rectangle in local coordinate space and an elevation. In most cases the elevation increment
should be 1 (default). The elevation increment for overlay widgets should be 25.

To reduce the complexities of the `set_layout()` and `draw()` functions, the `size` of the
layout MUST be at least the size of the `minimum` constraint. Otherwise it is possible
for calculations to underflow. Breaching the `maximum` constraint is less problematic for
these calculations and is allowed and expected to happen.

In the example below the `_layout` of the widget is updated by the `context` argument.
If the size was changed during the update then a new `_label_rectangle`
is calculated, in the widget's local coordinate system.

The child widget's `set_layout()` must be called even if the size has not changed, as the widget
may have been moved which is captured in the layout as well. As you can see the layout that is
passed to the child is calculated by transforming the context by the `_label_rectangle`.

```cpp
void set_layout(tt::widget_layout const &context) noexcept override
{
    if (_layout.store(context) >= tt::layout_update::size) {
        _label_rectangle = align(_layout.rectangle(), _label_widget->constraints().preferred, tt::alignment::middle_center);
    }

    _label_widget->set_layout(_label_rectangle * context);
}
```

### Drawing

This is a introduction to drawing a lot more information can be found in the [how to draw](how_to_draw.md)

The draw context has the following attributes:

 | attribute            | description                                                        |
 |:-------------------- |:------------------------------------------------------------------ |
 | `scissor_rectangle`  | The rectangle that is being drawn on the frame buffer.             |
 | `display_time_point` | The time when the drawing will appear on the screen.               |

The `scissor_rectangle` is used to only update the region of the window that needs to be redrawn.
When a widget calls the `request_redraw()` then the rectangle is merged with the window's redraw-rectangle, and
on the next frame the `scissor_rectangle` is calculated to include the current redraw-rectangle. It is allowed
to draw outside the `scissor_rectangle`, but this will not be visible.

The `display_time_point` includes the delays in the swap-chain for double/tripple buffering and
processing delays in the display device when supported by the operating system.

The `draw_context::draw_*()` methods all accept the layout as the first argument, this allows the other arguments
to the draw function to be in the local coordinate system. In certain cases a widget may want to make a copy of the layout
to temporarily change the clipping rectangle.

In the example below you can see that only when a widget is `visible` should the widget and its children be drawn.
The `overlap()` check compared the context's `scissor_rectangle` with the layout's `redraw_rectangle` only when they
partially or fully overlap should a widget draw its internals.

As you can see many values for the draw calls come from values taken from the current theme, and from the
context-sensitive colors which are based on the current theme's colors and may change based on the
state of the widget, like: keyboard focus, window active & mouse hover.

```cpp
void draw(tt::draw_context const &context) noexcept override
{
    if (visible) {
        if (overlaps(context, layout())) {
            context.draw_box(
                _layout,
                _layout.rectangle(),
                background_color(),
                foreground_color(),
                theme().border_width,
                tt::border_side::outside,
                theme().rounding_radius);
        }

        _label_widget->draw(context);
    }
}
```

Event handling
--------------

### Hitbox check
When you want your widget to receive mouse events you will first need to override
the `widget::hitbox_test()` function. This function is called by the operating
system to determine the type of cursor and if the window can be dragged or resized at that position.

ttauri at the same time uses this function to determine if this widget is directly underneath the
mouse cursor; at the highest elevation. It will then send mouse events directly to the widget, and
continue to track drag and exit events when the mouse leaves the widget.

The default `hitbox_test()` returns an empty `hitbox`, this means that the mouse pointer is not
hitting the widget, or the widget does not receive any mouse events at this time.

To handle situations where your widget is scrolled outside of a scroll view's aperture you should always use
the `layout().constains(position)` test which also checks the clipping rectangle.

You can call `widget::hitbox_test_from_parent()` on any of the widget's children that can be interacted with.
The `hitbox_test_from_parent()` will adjust the given position to the local coordinate system of the child widget.
You can also use this function to combine the `hitbox` results from several children.

```cpp
[[nodiscard]] tt::hitbox hitbox_test(tt::point3 position) const noexcept override
{
    if (visible and enabled and layout().contains(position)) {
        return {this, position, tt::hitbox::Type::Button};
    } else {
        return {};
    }
}
```

### Accepting keyboard focus
Similar to how `hitbox_test` will enable receiving mouse events for the widget,
overriding `widget::accepts_keyboard_focus()` will enable receiving keyboard events for the widget.

The `group` argument is a mask of reasons why this widget is being focused:

 | name    | description                                                                     |
 |:------- |:------------------------------------------------------------------------------- |
 | normal  | The user used (shift-) tab to select the next normal widget.                    |
 | menu    | The user used up/down-arrow to select the next widget in a pull down menu.      |
 | toolbar | The user used alt or left/right-arrow to select the next widget in the toolbar. |

A mouse click on a widget will cause `accepts_keyboard_focus()` to be called with all three
values set. This means that any widget that will accept focus from any of those in the table above
will also accept keyboard focus by a mouse click.

```cpp
[[nodiscard]] bool accepts_keyboard_focus(tt::keyboard_focus_group group) const noexcept override
{
    return visible and enabled and is_normal(group);
}
```

### Handling mouse events

### Handling keyboard events

### Handling command events

Child widgets
-------------
