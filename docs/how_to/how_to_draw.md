How to draw
===========

Drawing shapes
-------------
Most shapes are drawn using the box-shader, this shader does the following:
 * Draws a polygon between 4 vertices, 2 triangles.
 * Each vertex has a fill color and the polygon is smooth shaded
 * Each vertex has a border color: a border is drawn around the polygon and is smooth shaded
   at a given border width..
 * Each vertex has a corner radius: the polygon will be drawn with rounded corners.
 * The border and the polygon itself are anti-aliased.
 * Rendering of rounded corners and borders remain correct even when the polygon
   is an irreguar shape.
 * The polygon is clipped by an axis aligned rectangle.

The anti-aliased rounded rectangle shape is very powerfull and can be used to draw several shapes:
 * Rectangle
 * Rectangle with rounded corners
 * Circle
 * Slot
 * Line with flat or rounded end-points
 * Other 4 corner polygons with or without rounded corners.

The clipping rectangle also adds abilities, for example the toolbar-tab-button-widget
uses this to clip of the border on the bottom edge of the tab.

### Drawing rectangles

```

```


### Drawing rectangles with rounded corners

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
