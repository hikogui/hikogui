XXX Depth buffer can be used as per-primative clipping/stencil buffer.

# Render architecture
Vulkan is used as the backend for rendering windows.

```

    +----------+    +----------+
    |  Window  |    |  Window  |
    +----------+    +----------+
         |               |
         +---------------+
         |               |
     LCD Shader          |
         |               |
    +----------+       Hi DPI
    | HiDPI FB |       Bypass
    +----------+         |
         |               |
         +---------------+---------------+
         |               |               |
     Box Shader     Text Shader     Image Shader
                         |               |
                    +----------+    +---------+
                    |  Atlas   |    |  Atlas  |
                    +----------+    +---------+

```

## Window
Each window will have two swap-chain images assigned to it. The swap-chain
image are RGBA; Alpha is fixed to 1 and no need for a depth or stencil buffer.

When the window is HiDPI, then the Image, 

## Subpixel Pipeline
To anti-alias and potentially with LCD-sub-pixels a pipeline + shader
is used to super-sample a high-resolution frame buffer. This high resolution
frame buffer is treated as a 3x3 scaled window.

The high resolution framebuffer is a single 32 bit RGBA color attachment and a single
16 bit depth attachment.

## Image shader
The image shader takes a list of square-quads, with pointers inside a texture atlas to render.
Each polygon has the same size, and a image consists of a set of square-quads.

Due to the same size square-quads it is possible to manage the atlas dynamically, see the
seperate article about this.

## Solid polygon shader
A simple polygon shader for drawing simple objects quickly.

## Character shader
A character shader will render individual characters in high resolution using
signed distance fields.

A texture atlas is filled with glyphs that are added as needed at run-time.

## Text Shaping

Steps of text-shaping:
 - Start: with a list of style-graphemes in logical ordering. The style-grapheme contains the
   Unicode-NFC and each grapheme as a style. Latin automatic ligatures such as 'ffi' are illigal
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
 - Output 2: A list of carret locations, by index.
    - It is possible for a single index to have up to two locations, due to left-to-right and right-to-left switching.
    - It is possible to use this list to find the nearest carret location to the mouse cursor.
    - The carret location list also contains the carret height and slant. 


### Glyph Lookup algorithm

 1. Lookup grapheme's NFC code points in the code-point-to-font table and take the intersection of fonts.
 2. Lookup grapheme's NFD code points in the code-point-to-font table and take the intersection of fonts.
 3. Find the closest matching font compared to the font-style and fall-back to the Noto font.
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
 24:31 | Super-Family ID | Super font family id 0-255. 0 = Noto
    23 | Serif           |
    22 | Monospaced      | Serif + Monospace = Slab
    21 | Italic          | Italic / Oblique
    20 | Condensed       |
 16:19 | Weight          | See weight table below.
  8:15 | Optical size    | Design size in 0-255 pt
  0: 7 | Color index     |

The characteristics are read from the 'feat' table or parsed from the
ilename.

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
by the application at runtime. For example an audio application may use custom colours
for drawing the audio level meters.

### Font Styles
### Button shapes

