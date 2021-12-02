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

The second optional argument is an axis-aligned clipping rectangle
which will be intersected with the layout's clipping rectangle.
This can be used to specifically clip the shape, glyph or image being drawn.

Concepts
--------

### Coordinate system

The coordinate system for drawing is relative to the widget, including the elevation where the rectangle
is drawn on the z-buffer. The origin of the coordinate system is in the left-bottom corner of the pixel
in the left-bottom corner of the widget. The axes extent to the right, up and toward the user. 

The coordinates, widths and radii are in virtual pixels. On high resolution displays
there will be an integer scaling factor to map the virtual pixels onto physical pixels. In this library
when you read "pixel" without an explicit qualifier it is meant to be "virtual pixels".

### Font size

### Corners

The order of the corners in the `tt::quad`, `tt::quad_color` and `tt::corner_radii` are always:
left-bottom, right-bottom, left-top and right-top.


Drawing shapes
--------------

### Drawing rectangles

In the example below we draw a rectangle with a border and rounded corners:

```
void draw(tt::draw_context const &context) noexcept override
{
    auto const polygon = tt::quad(
        point3{10.0f, 10.0f, 0.0f},
        point3{60.0f, 10.0f, 0.0f},
        point3{10.0f, 60.0f, 0.0f},
        point3{60.0f, 60.0f, 0.0f}};
    auto const red = tt::color{1.0f, 0.0f, 0.0f, 1.0f};
    auto const blue = tt::color{1.0f, 0.0f, 0.0f, 1.0f};
    auto const border_width = 2.0f
    auto const corners = tt::corner_radii{3.0f, 3.0f, 3.0f, 3.0f};
    context.draw_box(_layout, polygon, red, blue, border_width, tt::border_side::inside, corners);
}
```

To get the rectangle to align to edges of pixels you want to specify the quad
as integer coordinates; and for the border side to specify either `inside` or `outside`
this will yield sharp borders.

The actual polygon send to the GPU is extended to include the part
of the border that falls outside the polygon and an extra physical pixel
for anti-aliasing.


### Drawing circles

When drawing circles among rectangular objects it is recommended to
draw the circle at a 1-3% larger radius, so that the shape seems to
be the same size and aligned to the flat edges of the rectangles. 


### Drawing lines

Drawing images
--------------


Drawing glyphs
--------------

Drawing text
------------

### Text shaping
