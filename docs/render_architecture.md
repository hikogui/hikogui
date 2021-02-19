
XXX Depth buffer can be used as per-primitive clipping/stencil buffer.

# Render architecture
Vulkan is used as the backend for rendering windows.

```

     +---------+
     | Window  |
     +---------+
          |      
      Tone Mapper
         | |
         | +---------------+
         |                 |
    +---------+            |
    | f16rgba |            |
    +---------+            |
         |                 |
     SDF Shader            |
        | |                |
        | +-------------+  |
        |               |  |
   +---------+      +---------+--------+
   |  Atlas  |      | f16rgba | Depth  |
   +---------+      +---------+--------+
                      |  |  |
                      |  |  +----------------------+
                      |  +-----------+             |
                      |              |             |
                 Image Shader    Flat Shader   Box Shader
                      |
                 +---------+
                 |  Atlas  |
                 +---------+





```

## Window
The swap-chain of the window will consist of RGBA images with alpha set to 1.

The window may either have the sRGB color space, or the extended-float16-sRGB
color space.

## Single pass, five sub-passes.
The whole render architecture is using a single pass with five sub-passes.

By using a single pass the shader of the sub-passes are able to efficiently
read from the frame-buffer. This frame buffer can be ephemeral and be located
completely in fast on-GPU-chip memory. This will make rendering on mobile
GPUs a lot more efficient compared to multi-pass architectures.

This shared frame buffer is in float-16 extended-sRGB, which allows for
HDR/WCG (High Dynamic Range / Wide Color Gamut). The frame buffer also
includes a depth image.

The five sub-passes are:
 - Flat Shader  - Render simple non-anti-aliased quads.
 - Box Shader   - Render anti-aliased rectangles with rounded corners.
 - Image Shader - Render anti-aliased texture mapped quads.
 - SDF Shader   - Render text-glyphs with sub-pixel-anti-aliasing.
 - Tone Mapper  - Convert HDR/WCG image to the reduced range of the display.

## Text Shaping

Steps of text-shaping:
 - Start: with a list of style-graphemes in logical ordering. The style-grapheme contains the
   Unicode-NFC and each grapheme as a style. Latin automatic ligatures such as 'ffi' are illegal
   and should never be composed into a single grapheme.
 - Unicode-bidirectional-algorithm: This will put the list of graphemes in
   left-to-right render order. The new graphemes are in the form style-index-grapheme, here
   the index points back to the original order.
 - Glyph lookup: Each style-index-grapheme is looked up and converted into a set of font-glyph-size-color-index.
   The search-priority algorithm will be described below.
 - Glyph morphing: Process a sequence of glyphs from the same font through the font's glyph morphing algorithms.
   The resulting font-glyph-size-color-index may contain fewer or more glyphs due to this morphing.
   This is also the place where ligatures like 'ffi' are constructed by the font itself.
 - Line breaking: based on a given width, extra line breaks are added to the text.
 - Advance and kerning: Each glyph is assigned a screen position based on the advance and kerning algorithms of
   the font.
 - Output 1: A list of vertex-points with screen-, texture- and color coordinates. And a index back to the original
   graphemes in logical ordering.
 - Output 2: A list of caret locations, by index.
    - It is possible for a single index to have up to two locations, due to left-to-right and right-to-left switching.
    - It is possible to use this list to find the nearest caret location to the mouse cursor.
    - The caret location list also contains the caret height and slant. 


### Glyph Lookup algorithm

 1. Lookup grapheme's NFC code points in the code-point-to-font table and take the intersection of fonts.
 2. Lookup grapheme's NFD code points in the code-point-to-font table and take the intersection of fonts.
 3. Find the closest matching font compared to the font-style.
 4. Open the matching font.
 5. Lookup glyphs matching the code points from the grapheme's (NFC primary, NFD secondary).
 6. If next grapheme's style is the same, then try looking up the code-points in the current font, otherwise start at 1.
 7. Otherwise start at 1. with the next grapheme.

A grapheme's code point is looked up into the code-point-to-fonts table.
he returned fonts are then matched with the font-style of the grapheme.

This font is cached based on the style, and any code point is first checked with this
cached font. If the code point is not found then the full lookup is done.



### Font styles
A font style is a 32 bit description of how a grapheme should
be displayed. It is only 32 bit so that it is accompanied with each
attributed-grapheme.

A font-style has the following characteristics:
 
  Bits | Name            | Description
 -----:|:--------------- |:-------------------------------------
 24:31 | Super-Family ID | Super font family id 0-255.
    23 | Serif           |
    22 | Monospaced      | Serif + Monospace = Slab
    21 | Italic          | Italic / Oblique
    20 | Condensed       |
 16:19 | Weight          | See weight table below.
  8:15 | Optical size    | Design size in 0-255 pt
  0: 7 | Color index     |

The characteristics are read from the 'feat' table or parsed from the
filename.

The font characteristic struct is ordered so that the nearest match
can be found using nearest unsigned integer comparison. The color index
should be ignored in such a case.

Weight table:

  Code | Value  | Description
 -----:| ------:|:------------
     0 |    100 | Thin / Hairline
     1 |    200 | Ultra-light / Extra-light
     2 |    300 | Light
     3 |    400 | Normal / Regular
     4 |    500 | Medium
     5 |    600 | Semi-bold / Demi-bold
     6 |    700 | Bold
     7 |    800 | Extra-bold / Ultra-bold
     8 |    900 | Heavy / Black
     9 |    950 | Extra-black / Ultra-black


## Themes
A theme consists of predefined colors, fonts and button shapes.
A theme is loaded from a json file. The theme directory is scanned for all themes
and from the preferences the user can select the current theme at runtime.

### Colors
#### Accent Color
#### Semantic Color
#### Custom Color
Custom colors are for application specific widgets. These colors may also be modified
by the application at runtime. For example an audio application may use custom colors
for drawing the audio level meters.

### Font Styles
### Button shapes

## Box Pipeline
The box-pipeline is designed to draw axis-aligned quads with:
 - A fill color.
 - An anti-aliases border of a certain thickness and color.
 - An drop shadow around the border of a certain color.
 - An inlay shadow inside the border of a certain color.
 - Rounded and cut corners.

By using "texture" coordinates the fragment shader knows the distance towards an edge.
Together with a factor to convert the texture coordinate to pixel distance from
each edge.
