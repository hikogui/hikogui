Color
=====

Extended Linear sRGB
--------------------
Absolute color values uses the extended-linear-sRGB color space internally in
the application, the ttauri library and its shaders. This allows for HDR (high dynamic range)
and high gamut colors while remaining backward compatible with normal sRGB and rec709 colors.

The extended linear sRGB uses the ITU-R BT.709 color primaries and white-point, but uses
a linear transfer function, where the value (0.0, 0.0, 0.0) is 0 nits black and
(1.0, 1.0, 1.0) is 80 nits white.

Values above 1.0 for high dynamic range, and values below 0.0 for high gamut are
allowed.

Alpha
-----
Alpha is encoded as a linear floating point number between 0.0 (transparent) and 1.0 (opaque).

Inside the application and ttauri-library the color values are NOT pre-multiplied with the alpha value.
However the shaders may pre-multiply alpha internally for optimization or color accuracy reasons.

Coverage blending
-----------------
When compositing glyphs onto background we use a mask with coverage value, of how much the glyph has
covered that specific pixel.

If you convert this coverage value 0% - 100% directly into an alpha value of 0.0 and 1.0 you will
find that linear compositing of the text color on the background will use different result when
compositing dark on light vs light on dark. Where the weight of the glyph seems to change.

In fact linear compositing is actually incorrect and compositing should be done by keeping in mind
the perceptional non-linear lightness curve and the actual color values of the foreground and background.

The perceptional curve is cubic, as an optimization a square perceptional curve seem to be visually good
enough to handle fonts.

The following function is used for subpixel-compositing, with a coverage value `a` for each subpixel.

```
vec3 mix_perceptual(vec3 x, vec3 y, vec3 a)
{
    vec3 r = mix(sqrt(x), sqrt(y), a);
    return r * r;
}
```


Semantic colors
---------------
TTauri uses themes for defining color values for a design, which are used a preferences for the
user but also for different theme modes like dark and light and accessibility modes.

When an application needs a color it would select this color from the theme, below the complete
list of colors.

  id     | Name        | Description
  ------:|:----------- |:-----------
         | Gray 1 (BG) |
         | Gray 2      |
         | Gray 3      |
         | Gray 4      |
         | Gray 5 (M)  |
         | Gray 6      |
         | Gray 7      |
         | Gray 8      |
         | Gray 9 (FG) |

         | Blue        |
         | Green       |
         | Indigo      |
         | Orange      |
         | Pink        |
         | Purple      |
         | Red         |
         | Teal        |
         | Yellow      |

         | label 1     | The color used for foreground text.
         | label 2     | The color used for foreground text.
         | label 3     | The color used for foreground text.
         | label 4     | The color used for foreground text.

         | fill 1      |
         | fill 2      |
         | fill 3      |
         | fill 4      |

         | border 1      |
         | border 2      |
         | border 3      |
         | border 4      |




