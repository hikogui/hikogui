# TTauri Coordinate System

## Coordinates
Window, image and path coordinates are in the number of pixels.
X-axis pointing right, Y-axis pointing up.
The origin (0,0) of a window, image and path start at the bottom-left
corner. The center of the bottom-left pixel in a window and image is
at (0.5, 0.5).

Positive rotation is counter clockwise. Vertices of front faces are
specified in counter clockwise order.

## Glyph/Font coordinates
Glyph coordinates are in the number of Em units.
X-axis pointing right, Y-axis pointing up.
The origin (0,0) is on the base line and left side bearing.
Vertices of front faces are specified in counter clockwise order.

## Corners
Corners are enumerated as follows:
 - 0: left bottom
 - 1: right bottom
 - 2: right top
 - 3: left top

This is the same order as the corner-shape vector.

## Quads
The vertex index order is always:
 - 0, 1, 2, 0, 2, 3

As illustrated below:

```
3 <--- 2
|    / ^
| B /  |
|  / A |
v /    |
0 ---> 1
```

