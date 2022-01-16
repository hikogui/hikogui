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

### Corners

The order of the corners in the `tt::quad`, `tt::quad_color` and `tt::corner_radii` are always:
 - left-bottom,
 - right-bottom,
 - left-top and
 - right-top.

### Edges

The order of in the `tt::margins` are always;
 - bottom
 - left
 - top
 - right

Drawing shapes
--------------

### Drawing rectangles

In the example below we draw a rectangle with a border and rounded corners:

```
void draw(tt::draw_context const &context) noexcept override
{
    auto const polygon = tt::quad{
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

In the example below we draw a circle with a border:

```
void draw(tt::draw_context const &context) noexcept override
{
    auto const polygon = tt::circle{point3{35.0f, 35.0f, 0.0f}, 25.0f};
    auto const red = tt::color{1.0f, 0.0f, 0.0f, 1.0f};
    auto const border_width = 2.0f
    context.draw_circle(_layout, polygon, line_width, red, tt::line_end_cap::round, tt::line_end_cap::round);
}
```

`tt::draw_context::draw_circle()` is a convenience-function for
`tt::draw_context::draw_box()` A circle is a square with rounded corners
with the corner diameter set to the height/width of the square.

When drawing circles among rectangular objects it is recommended to
draw the circle at a 1-3% larger radius, so that the shape seems to
be the same size and aligned to the flat edges of the rectangles. 


### Drawing lines

In the example below we draw a line with rounded end points:

```
void draw(tt::draw_context const &context) noexcept override
{
    auto const line = tt::line{
        point3{10.0f, 10.0f, 0.0f},
        point3{50.0f, 50.0f, 0.0f}};
    auto const red = tt::color{1.0f, 0.0f, 0.0f, 1.0f};
    auto const blue = tt::color{1.0f, 0.0f, 0.0f, 1.0f};
    auto const line_width = 2.0f
    context.draw_line(_layout, line, red, blue, border_width, tt::border_side::inside);
}
```

`tt::draw_context::draw_line()` is a convenience-function for
`tt::draw_context::draw_box()` A line is a thin rectangle with rounded corners
with the corner diameter set to the width of the line.

When drawing horizontal or vertical lines you may want to position the line so
that it goes through the center of the pixels to get a sharp line. The
origin of the coordinate system is in the left-bottom corner of the most left
bottom pixel of a widget.

Drawing images
--------------

There are three steps for drawing an image:
 - Loading an png-image file from disk.
 - Uploading the image to the GPU.
 - Drawing the polygons to display the image.

Here we load the `mars3.png` file from a resource directory from the constructor:

```cpp
drawing_widget(tt::gui_window &window, tt::widget *parent) noexcept :
    widget(window, parent), _image(tt::URL("resource:mars3.png")) {}
```

During `set_constraints()`  we try to construct a `tt::paged_image`. In the
`set_constraints()` function this may fail when the window is still being created;
therefor we keep reconstraining until it succeeds.

This version of the `tt::paged_image{}` constructor also directly uploads the
image to the GPU. It is also possible to make a `tt::paged_image` with just
a width and a height and then upload the image at a later time.


```cpp
tt::widget_constraints const &set_constraints() noexcept override
{
    _layout = {};
    if (_image_was_modified.exchange(false)) {
        if (not (_image_backing = tt::paged_image{window.surface.get(), _image})) {
            // Could not get an image, retry.
            _image_was_modified = true;
            request_reconstrain();
        }
    }
    return _constraints = {{100, 100}, {150, 150}, {400, 400}, theme().margin};
}
```

Drawing glyphs
--------------

Drawing text
------------

### Text shaping
