# TTauri Coordinate System

## Coordinates
Window, image and path coordinates are in the number of pixels.
X-axis pointing right, Y-axis pointing up.
The origin (0,0) of a window, image and path start at the bottom-left
corner. The center of the bottom-left pixel in a window and image is
at (0.5, 0.5).

Positive rotation is counter clockwise. Vertices of front faces are
specified in counter clockwise order.

A window has a scaling factor, to convert the size of a point to a pixel.
i.e. a scaling factor of 2.0 means: 2.0 pixels per point.

## Glyph/Font coordinates
Glyph coordinates are in the number of Em units.
X-axis pointing right, Y-axis pointing up.
The origin (0,0) is on the base line and left side bearing.
Vertices of front faces are specified in counter clockwise order.
