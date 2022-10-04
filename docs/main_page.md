HikoGUI {#mainpage}
==================

HikoGUI is a cross platform C++ GUI library.

Manuals
-------
 - [How to use widgets](how_to/how_to_use_widgets.md)
 - [How to log](how_to/how_to_log.md)
 - [How to use preferences](how_to/how_to_use_preferences.md)
 - [How to make custom widgets](how_to/how_to_make_custom_widgets.md)
 - [How to draw](how_to/how_to_draw.md)

Subsystems
----------
Documentation for the HikoGUI sub-systems can be found in the:
 - [Module](modules.html) ![new](media/images/new.png) and
 - [Related Pages](pages.html) tabs.

Features
--------

### Box drawing

Most drawing is done through the box-drawing shader.
The box-drawing shader can draw an anti-aliased box with:

 - a background color mixed from the 4 vertices,
 - a border color mixed from the 4 vertices,
 - a border width,
 - a corner radius for each corner, and
 - a clipping rectangle to cut part of the box.

This primitive can be used to draw different shapes, like:

 - normal rectangles,
 - rounded rectangles,
 - slots, and
 - circles.

### Text drawing

Glyphs are drawn by the GPU using a signed-distance-field shader.
This shader is able to render glyphs with subpixel-anti-aliasing
without using a post-processing filter.

Glyphs are lazily and asynchronous, added to the texture-atlas
when needed. By adding the glyphs as signed distance fields to
the texture atlas a glyph needs to be added only once to be usable
for displaying at any size.

Performance
-----------

HikoGUI is designed for low latency interactive applications.

For this reason, we have the following design considerations:

 - Widgets need to be able to animate at 60 fps.
 - CPU and GPU usage when drawing at 60 fps need to remain low.
 - Monitored data needs to be reflected in the user interface
   with at most one frame delay.
 - During drawing the widget can use predictive algorithms to
   determine what should be shown to the user at the display time.
   When for example showing the current time on the display.

The following is used to implement the requirements:

 - Use game-like redraw loop running at the current system's
   frame rate. Immediately reflecting updated data. And caching
   size-constraining, layout, text-shaping, and other expensive
   pre-draw operations.
 - Partial drawing, widgets will only draw anything that falls
   within the current scissor rectangle. The scissor rectangle
   allows the GPU to discard drawing outside it to improve
   drawing speed.
 - All drawing is done by passing vertices to different
   shaders for GPU accelerated drawing of:
   rounded-rectangles, pixmap-images and text-glyphs.
