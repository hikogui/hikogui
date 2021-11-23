How to draw
===========

Drawing is done through the `tt::draw_context` object that is passed to
`tt::widget::draw()` when drawing a frame. The `tt::draw_context` has
several `draw_*()` member functions that allows drawing of shapes,
glyphs, text and images.

The first argument to those `draw_*()` functions is an reference to the
widget's `tt::widget_layout` object. This object was stored by the
`tt::widget::set_layout()` function and contains the to-window
transformation matrix and clipping rectangle used when drawing.

The transformation matrix is used so that drawing can be done
in the local cordinate system of the widget. Where the left bottom
corner of the widget is the origin. With the axis:
 * x increasing to the right,
 * y increasing to the top, and
 * z increasing toward the user.

The second optional argument is an axis-aligned clipping rectangle
which will be intersected with the layout's clipping rectangle.
This can be used to specifically clip the shape, glyph or image being drawn.



Drawing shapes
--------------

### Drawing rectangles and other quads

The `tt::draw_context::draw_box(

```
void draw(tt::draw_context const &context) noexcept override
{
    auto const q = tt::quad(point3{3, 0, 0}, point3{
    context.draw_box(_layout, q, f)
}
```


### Drawing quads with rounded corners

### Drawing circles

When drawing circles among rectangular objects it is recommended to
draw the circle at a 1-3% larger radius, so that the shape seems to
be the same size and aligned to the flat edges of the rectangles. 

### Drawing slots

When drawing slots among rectangular objects it is recommended to
extent the flat edge by 1-3% of the diameter of the rounded edge,
so that the shape seems to be the same size and aligned to the flat
edges of the rectangles. 


### Drawing lines

Drawing images
--------------


Drawing glyphs
--------------

Drawing text
------------

### Text shaping
