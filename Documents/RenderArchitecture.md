# Render architecture
Vulkan is used as the backend for rendering windows.

## Window
Each window will have two swap-chain images assigned to it. The swap-chain
image are RGBA; Alpha is fixed to 1 and no need for a depth buffer.

To anti-alias and potentially LCD-sub-pixel anti-alias a pipeline + shader
is used to super-sample a high-resolution frame buffer. This high resolution
frame buffer is treated as a 3x3 scaled window.

Since the high-resolution frame buffer is shared between windows, each window
will be rendered in turn.

## High-resolution Framebuffer
The high resolution framebuffer is a single RGBA color attachment and a single
32 bit float depth attachment.

This framebuffer is shared between all windows (to reduce memory usage),
t should therefor be at least 3x3 time the size of the largest window. 

There are three different pipeline + shaders run on the frame buffer.

## Image shader
The image shader takes a list of square-quads, with pointers inside a texture atlas to render.
Each polygon has the same size, and a image consists of a set of square-quads.

Due to the same size square-quads it is possible to manage the atlas dynamically, see the
seperate article about this.

## Solid polygon shader
A simple polygon shader for drawing simple objects quickly.

## Character shader
A character shader will render individual characters in high resolution using multi-colour
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
 - Line breaking: based on a given width, extra line breaks are added to the text.
 - Advance and kerning: Each glyph is assigned a screen position based on the advance and kerning algorithms of
   the font.
 - Output 1: A list of vertex-points with screen-, texture- and color coordinates. And a index back to the original
   graphemes in logical ordering.
 - Output 2: A list of carret locations, by index.
    - It is possible for a single index to have up to two locations, due to left-to-right and right-to-left switching.
    - It is possible to use this list to find the nearest carret location to the mouse cursor.
    - The carret location list also contains the carret height and slant. 


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

