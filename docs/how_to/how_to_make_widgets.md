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

