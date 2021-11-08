How to make widgets
===================

Visual
------
A widget is rendered in three steps:
 1. `widget::set_constraints()` is called to determine the minimum, preferred & maximum size and margin of the widget.
 2. `widget::set_layout()` is called to set the size and coordinate-transformations for the widget.
 3. `widget::draw()` is called to (partially) draw the widget.

Each of the three steps is optional, with increasing order of likeliness to be called on each
vertical-sync.

### Constraints
When the window is first opened or when a widget requests a reconstraine; a depth-first query is done
on all the widget on a window to request the size and other attributes needed for laying-out the widgets.

Currently the constrain attributes are:
 * minimum-size: the guaranteed smallest size the widget will be layed out as.
 * preferred-size: used to determine the initial size of the window, and initial layout of container widgets.
 * maximum-size: the largest size a widget will be; unless during layout another widget's minimum-size will force
                 this widgets size beyond the maximum size.
 * margin: The distance between sibling widgets, is the maximum of the margins of the widgets.

### Layout

### Drawing

### Context sensitive colors

Event handling
--------------

### Hitbox check

### Accepting keyboard focus

### Handling mouse events

### Handling keyboard events

### Handling command events

Child widgets
-------------

