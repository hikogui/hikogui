# TTauri Icon File Format

The TTauri Icon File Format is used for storing a single multicolor
icon in a simple binary format.

All values are in little endian byte order.

Paths in a icon are drawn in-order following the painter's algorithm.

  type                 | Name         | description
 :-------------------- |:------------ |:----------
  char[4]              | magic_number | "TTIC" Magic Number
  uint16_t             | nr_paths     | Number of paths
  path[nr_paths]       | paths        | Paths


## Path
If both fill and stroke are non-transparent, then the path will be filled
first followed by stroke.

  type                 | Name         | description
 :-------------------- |:------------ |:----------
  color                | fill_color   | Fill color index.
  color                | stroke_color | Stroke color index.
  fixed1_13_t          | stroke_width | Width of the stroke + line join style flag
  uint16_t             | nr_contours  | Number of contours
  contour[nr_contours] | contours     | Contour

Line Join Style flag:
 * true = Bevel
 * false = Miter

## Contour
A closed contour is drawn counter clockwise.

  type             | Name      | description
 :---------------- |:--------- |:----------
  uint16_t         | nr_points | Size of the contour data.
  point[nr_points] | points    | Points

## Point
Very much like how glyphs inside a font file are encoded, the points describe coordinates in em space.
The origin is at the bottom left of the em box; on the base-line, on the leading edge.
Positive values go up and to the right.

  type        | Name | description
 :----------- |:-----|:----------
  fixed1_13_t | x    | x, flag A
  fixed1_13_t | y    | y, flag B

The flags identify the type of point.

  flag A | flag B | description
 :------ |:------ |:------------
  0      | 0      | Anchor Point.
  1      | 0      | Cubic Bezier control point 1.
  0      | 1      | Cubic Bezier control point 2.
  1      | 1      | Quadratic Bezier control point.

As optimizations some points may be omitted:
 * An anchor point is automatically inserted at the midpoint between two
   quadratic control points.
 * A cubic control point 1 is automatically inserted between an anchor point
   and a cubic control point 2 by reflecting the previous control point 2
   through the anchor point.
 * Two anchor points are interpreted as a line.

## fixed1_14_t
This is a signed fixed point number, encoding a number between -2.0 and almost 2.0.
The following formula is used to convert a number to this fixed point format: `floor(x * 16384)`
this integer is then encoded as a int16_t.

## fixed1_13_t
This is the same format as fixed1_14_t, but the least-significant-bit is used as a boolean flag.

## Color
Colors are encoded as wide-gamut linear scRGB colors.

Linear floating point color component values are converted by `floor(x * 8192) + 4096`.
The full range is of color component values is -0.5 - 7.5; the range between 0.0 - 1.0
are within the sRGB gamut.

The alpha value is encoded with: `alpha * 65535`. The color component are not pre-multiplied with
the alpha value.

  type     | description
 :-------- |:----------
  uint16_t | Red component
  uint16_t | Green component
  uint16_t | Blue component
  uint16_t | Alpha component
