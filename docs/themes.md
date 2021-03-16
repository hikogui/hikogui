GUI Themes
==========

Color Style
-----------

Each color style has a combination of attributes that should be used together:

 - Background-color
 - Foreground-color
 - Accent-color
 - Shadow-color
 - Focus-color: Color of a focus ring.

Text Style
----------

Styles:

 - Default
 - Dim
 - Bright
 - Heading
 - Help: For help text
 - Warning
 - Error

Attributes:

 - Color-style
 - Font (uses foreground-color)
 - Font-size
 - Underline (uses accent-color)
 - Double-Underline (uses accent-color)
 - Strike-through (uses accent-color)
 - Wave (uses accent-color)
 - Drop shadow (uses shadow-color)

GUI Element
-----------

GUI Elements may be nested and should copy the color-palette from the parent.
The use may configure user elements to have a different color-palette.

Styles:

 - Default
 - Important
 - Compact: For use in panels

State:

 - Normal
 - Focus
 - Disabled

Attributes:

 - Color-style
 - Text-style
 - Corner-radius
 - Corner-style
