Render architecture
===================

Vulkan is used as the backend for rendering windows.


```

     +---------+
     |Swapchain|
     +---------+
          |
          |
     Tone Mapper
          |
     +---------+--------+
     | f16rgba | Depth  |
     +---------+--------+
      |  |  | |
      |  |  | +----------------------------------+
      |  |  +-----------------------+            |
      |  +------------+             |            |
      |               |             |            |
  Box Shader     Image Shader  SDF Shader    Hole Punch
                      |             |
                 +---------+   +---------+
                 |  Atlas  |   |  Atlas  |
                 +---------+   +---------+

```

Window
------

The swap-chain of the window will consist of RGBA images, with the format
that can be handled by the operating system. This may be a 8-bit sRGB color
space format or an extended float 16 sRGB.

TTauri will use alpha blender to render the user interface on top of the swap
chain. This means an application can use render an image into the swap-chain before
this. See the hole-punch shader on how to set the alpha channel on a region.

Single pass, five sub-passes
-----------------------------

The whole render architecture is using a single pass with five sub-passes.

By using a single pass the shader of the sub-passes are able to efficiently
read from the frame-buffer. This frame buffer can be ephemeral and be located
completely in fast on-GPU-chip memory. This will make rendering on mobile
GPUs a lot more efficient compared to multi-pass architectures.

This shared frame buffer is in float-16 extended-sRGB, which allows for
HDR/WCG (High Dynamic Range / Wide Color Gamut). The frame buffer also
includes a depth image.

The five sub-passes are:

 - Box Shader   - Render anti-aliased rectangles with rounded corners.
 - Image Shader - Render anti-aliased texture mapped quads.
 - SDF Shader   - Render text-glyphs with sub-pixel-anti-aliasing.
 - Hole punch   - Modified alpha value to control the tone mapper.
 - Tone Mapper  - Convert HDR/WCG image to the reduced range of the display.

Text Shaping
------------

### Glyph Lookup algorithm

 1. Search grapheme's code points in the font
 2. Lookup grapheme's decomposed code points in the font
 3. Find the closest matching font compared to the font-style.
 4. Open the matching font.
 5. Lookup glyphs matching the code points from the grapheme's
 6. If next grapheme's style is the same, then try looking up the code-points in the current font, otherwise start at 1.
 7. Otherwise start at 1. with the next grapheme.

A grapheme's code point is looked up into the code-point-to-fonts table.
The returned fonts are then matched with the font-style of the grapheme.

This font is cached based on the style, and any code point is first checked with this
cached font. If the code point is not found then the full lookup is done.


Themes
------

A theme consists of predefined colors, fonts and shape sizes.
A theme is loaded from a json file. The theme directory is scanned for all themes
and from the preferences the user can select the current theme at runtime.

### Colors

#### Accent Color

#### Semantic Color

### Font Styles

