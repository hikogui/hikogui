# TTauri Geometry System

## Data Types
### vec
The `vec` class is a 4D homogenious coordinate.  
Internally this is a `__m128` on x64 CPUs, where the bits are:
 - x [31:0]
 - y [63:32]
 - z [95:64]
 - w [127:96]

The `vec()` constructor will default with w=0.0. There are also the
`vec::point()` and `vec::color()` factories which default w=1.0.

### mat
The `mat` class is a 4x4 homogenious transformation matrix in column-major
order. Internally this a `vec` for each column.

Vector * matrix multiplications are performed as if the vector is a column.

### aarect
The `aarect` class is a 2D axis-aligned rectangle.

Internally this is a `vec` where:
 - x - left-bottom point x.
 - y - left-bottom point y.
 - z - right-top point x.
 - w - right-top point y.

## Coordinates
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
the DrawContext::drawBoxIncludingBorder() function to position the border's
edge to the edge of the given rectangle, if the rectangle is rounded to
integer coordinates; the rectangle, border and pixel will share the same edge.

At DPI-scale of 1, it is assumed that the display has a 84 DPI.
This is the middle ground between Window's default 96 DPI and macOS's
default 72 DPI. The ShapedText class will scale to 84 DPI automatically,
further DPI scaling will be handled by the DrawContext.



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

