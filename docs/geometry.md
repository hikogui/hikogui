TTauri Geometry System
======================

Low level geometry types
------------------------

### numeric_array<T,N>
The `numeric_array` is an array of numbers, with many mathematical operations
on the array of numbers. The numeric_array is designed to be useable in
constexpr and easilly to vectorize by the optimizer.

### f32x4
The `f32x4` is an `numeric_array<float,4>` many of the operations on a `f32x4`
are hand optimized using the intel intrinsics on SSE registers.

High level geometry type
------------------------

### geo::vector<D>
A vector is a direction and distance.
 
When transforming a vector, only scale, rotation and shear have effect.

Both `vector2` and `vector3` are implemented as a `f32x4` homogenious 4D coordinate with w = 0.0.

### geo::point<D>
A point is a location in space.
 
A point can be transformed in the same way as a vector and also be translated.

Both `point2` and `point3` are implemented as a `f32x4` homogenious 4D coordinate with w = 1.0.

### geo::extent<D>
An extend is a width, height and depth.
 
A extent can be transformed like a vector.
 
Both `extent2` and `extent3` are implemented as a `f32x4` homegeniuos 4D coordindate with w = 0.0.

### corner_shapes
Corner shapes are 4 floating point numbers one for the corner in the left-bottom, right-bottom,
left-top and right-top corner. Each number has the following meaning:

 - 0.0 Sharp corner,
 - >0.0 Radius of a rounded corner,
 - <0.0 Radius of a cut corner.

### color
A 4D red, green, blue and alpha values. A `color` can be transformed like a `vector3`, in this
case the alpha value is ignored and copied into the result.

For more information see: [color](color.md).

### rectangle
A `rectangle` is a closed plane in three dimensions.

A `rectangle` can be transformed like a `point3`.

It should be implemented as a `point3` in the left-bottom corner and two `vector3`s to the upper and right corners.
However it is currently implemented as 4 points one for each corner.

### axis_aligned_rectangle
The `axis_aligned_rectangle` class is a 2D axis-aligned rectangle.

When transforming an axis aligned rectangle in 3D or with rotation the result will be a normal `rectangle`.
A rectangle can be converted back to an axis_aligned_rectangle, as a bounding rectangle around the transformed rectangle.

a axis_aligned_rectangle is implemented as a `f32x4` where:
 - x - left-bottom point x.
 - y - left-bottom point y.
 - z - right-top point x.
 - w - right-top point y.


Transformation types
---------------------

### geo::identity
An identity transform does not

### geo::translate<D>

### geo::scale<D>

### geo::rotate<D>
 
### geo::matrix<D>
The `mat` class is a 4x4 homogeneous transformation matrix in column-major
order. Internally this a `vec` for each column.

Vector * matrix multiplications are performed as if the vector is a column.

### geo::transform




Coordinates
-----------
All origins (0, 0, 0) are at the bottom-left-far corner. With the x-axis pointing
right, y-axis pointing up, z-axis pointing near.

This is true for:
 - Window
 - Images
 - Paths
 - Font-glyphs

### Window and Widget-surface coordinates
The local coordinates in a window and widgets are in points.
With the origin in the corner of the left bottom most point.

The local x,y-coordinates are multiplied by the DPI-scale of the
window. The DPI-scale is always an integer, which allows much easier
alignment of lines to actual pixels.

The alignment of borders to pixels is the responsibility of the
widget that draws itself. The widget has access to
the draw_context::drawBoxIncludingBorder() function to position the border's
edge to the edge of the given rectangle, if the rectangle is rounded to
integer coordinates; the rectangle, border and pixel will share the same edge.

At DPI-scale of 1, it is assumed that the display has a 84 DPI.
This is the middle ground between Window's default 96 DPI and macOS's
default 72 DPI. The ShapedText class will scale to 84 DPI automatically,
further DPI scaling will be handled by the draw_context.

### Window depth
Z-coordinate for a window is between 0.0 (far) to 100.0 (near).
For better precision we use a reverse-z method, to combine
1/z together with float with linear precision.

 - The window-widget is set to depth 0.0.
 - Each nested widget, which needs to draw itself, is 1.0 nearer.
 - Widgets which extends across other widgets, such as a combo-box-widget
   will be 25.0 nearer.

### Image coordinates
The coordinates in a image are in pixels. With the center of the left bottom
pixel having the coordinates (0.5, 0.5).

### Path coordinates
The coordinates in a path are free from scale and origin, when rendering a
path to an image, the path should be first be transformed to image coordinates.

### Glyph/Font coordinates
Glyph coordinates are in the number of Em units.
X-axis pointing right, Y-axis pointing up.
The origin (0,0) is on the crossing of the base line and left side bearing.
Vertices of front faces are specified in counter clockwise order.

## Corners
Corners are enumerated as follows:
 - 0: near bottom left
 - 1: near bottom right
 - 2: near top left
 - 3: near top right
 - 4: far bottom left
 - 5: far bottom right
 - 6: far top left
 - 7: far top right

When corners are passing as 4D vectors:
 - x = bottom-left
 - y = bottom-right
 - z = top-left
 - w = top-right

## Triangles
A front facing triangle has the vertex ordered in counter-clockwise direction.

## Quads
Quads are defined as two front facing triangles.
The vertex index order is: 0, 1, 2, 2, 1, 3

As illustrated below:

```
2 <--- 3
| \    ^
|  \ B |
| A \  |
v    \ |
0 ---> 1
```

