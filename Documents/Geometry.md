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
right, y-axis pointing up, z-axis pointing away.

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

What is the default DPI? Windows says 96dpi, macOS says 72dpi. Mean is 84dpi.
Should fonts be scaled properly, while line drawing are integer scaled? This
maybe possible since we need to size the widgets based on shaped-text from different
languages anyway.

### Window depth
Z-coordinate for a window is between 0.0 (far) to 1.0 (near).
For better precission we use a reverse-z method, to combine
1/z together with float with linear precission.

 - The window-widget is set to depth 0.0.
 - Each nested widget is 0.001 nearer.
 - Widgets which extends accross other widgets, such as a combo-box-widget
   will be 0.1 nearer.
 - A utlity sub-window is set to a depth of 0.25
 - A sheet over the window is set to a depth of 0.5
 - A modal dialogue is set to a depth of 0.75
 - A privacy-overlay/screen-saver is set to a depth of 0.95

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

