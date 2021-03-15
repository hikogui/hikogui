Perceptive correct anti-aliasing
================================

Terms
-----

### Linear-sRGB color space

Linear-sRGB is an inaccurate term describing a color space based on
the sRGB color primaries and white point, but with a linear transfer function.

The color component values have a range between 0.0 and 1.0 and are represented
as floating point values. In certain cases values outside the 0.0 to 1.0 range
are allowed to handle luminance values beyond 80 cm/m2 for HDR, or negative values
which represent colors outside the triangle of the sRGB color primaries.

This is the color space that is used inside fragment shaders on a GPU.

### Luminance (Y)

The luminance is a physical linear indication of brightness.

In this paper we use the range of luminance values between 0.0 and 1.0 to
be mapped linear to 0 cd/m2 to 80 cd/m2; which is the standard range of
sRGB screen-luminance-level.

The luminance value is calculated from the linear-sRGB values as follows:

```
Y = 0.2126 * R + 0.7152 * G + 0.0722 * B
```

My intuition of luminance calculations in high-gamut extended-sRGB color space
is that the luminance is always positive. Since if a color outside the standard-sRGB
triangle is selected at least one of the components must be a positive enough
value to force the luminance to be above zero. Having only positive values is important
for taking the square root, see the next section.

<https://en.wikipedia.org/wiki/Relative_luminance>

### Lightness (L)

Lightness is the perceptual uniform indication of brightness.

In this paper we use the range of lightness values between 0.0 and 1.0 to
be mapped non-linear to 0 cd/m2 to 80 cd/m2.  which is the standard range of
[sRGB](https://en.wikipedia.org/wiki/SRGB) screen-luminance-level.

The lightness and luminance value can be translated as follows:

```
L = sqrt(Y)
Y = L * L
```

The formula above is an approximation made in 1920, the current well accepted
approximation is closer to cube-root curve with a linear section for dark values used
in the 1976's CIELAB color space. However the square-root curve is accurate enough
for the use-case of anti-aliasing glyphs and is much faster in hardware and easier to
use when doing the calculations for this paper.

<https://en.wikipedia.org/wiki/Lightness>


The problem
-----------

As everyone knows alpha compositing of two images must be done in the linear color
space or there will be color and/or brightness shifts over the range of alpha values.

If however we linearly convert the pixel coverage of an anti-aliased line
to an alpha value between 0.0 and 1.0 we get the issue that the perceived
width of the line is different between light-on-dark or dark-on-light.

Below we calculate what we will perceptional see when anti-aliasing a vertical
line of two pixels wide centered at x=2.25. On the left side a white line
on black background and on the right side the black line on a white background.

```
           white on black-background    black on white-background
          +----+----+----+----+----+   +----+----+----+----+----+
coverage  | 0% |25% |100%|75% | 0% |   | 0% |25% |100%|75% | 0% |
          +----+----+----+----+----+   +----+----+----+----+----+

          +----+----+----+----+----+   +----+----+----+----+----+
alpha     |0.0 |0.25|1.0 |0.75|0.0 |   |0.0 |0.25|1.0 |0.75|0.0 |
          +----+----+----+----+----+   +----+----+----+----+----+

          +----+----+----+----+----+   +----+----+----+----+----+
luminance |0.0 |0.25|1.0 |0.75|0.0 |   |1.0 |0.75|0.0 |0.25|1.0 |
          +----+----+----+----+----+   +----+----+----+----+----+

          +----+----+----+----+----+   +----+----+----+----+----+
lightness |0.0 |0.5 |1.0 |0.87|0.0 |   |1.0 |0.87|0.0 |0.5 |1.0 |
          +----+----+----+----+----+   +----+----+----+----+----+
```

The perceived line width of "white on black-background" is: `width = 0.0 + 0.5 + 1.0 + 0.87 + 0.0 = 2.37`

The perceived line width of "black on white-background" is: `width = 5 - (1.0 + 0.87 + 0.0 + 0.5 + 1.0) = 1.63`

As you can see the perceived width of the line is significant different
especially when anti-aliasing objects which are thin, such as the lines
on a glyph.

The solution
------------

There are some papers that mention the problem of the change of apparent thickness
of glyphs when rendering black text or white text. Most of those papers go on
to explain this stems from not doing linear compositing, or explaining that the font
was designed for black on white.

However, as you see from the calculations in the previous section this has nothing to
do with the design of a font, because it shows up even when rendering a single line.
Also, if you would render the font without anti-aliasing these problems no longer
exist, such as when rendering with a high resolution printer.

Problems like this even happen with physical anti-alias filters, such as when using
an anti-alias glass in front of an image sensor, which scatter the photons over range
of pixels. It often shows as a reduction of contrast of textured images and on sharp edges,
normally this is solved by increasing the contrast of high frequency content, such
as using a sharpen-filter, or with professional cameras using an equalizer tuned on
the whole optical system.

Since with computer graphics we have control over anti-aliasing itself
we can make the algorithm mix the foreground and background color on a
perceptional uniform gradient.

The calculation below shows how to convert an anti-alias-pixel-coverage value
to an alpha value, when this pixel coverage is in a perceptional uniform space.

First we need to calculate the foreground and background lightness, based on the
final composite of the foreground and background color using the two alpha values
0.0 and 1.0. This definition will allow for semi-transparent foreground colors.

```
Yfront = 0.2126 * Rfront + 0.7152 * Gfront + 0.0722 * Bfront
Yback  = 0.2126 * Rback + 0.7152 * Gback + 0.0722 * Bback
Lfront = sqrt(Yfront)
Lback  = sqrt(Yback)
```

By mixing the foreground and background lightness using the coverage value, we
now have the target lightness for that coverage value.

```
Ltarget = mix(Lback, Lfront, coverage)
```

We can convert this target lightness to a target luminance, which can then be used to find
the alpha value needed to reach that target from the foreground and background luminance.
If the luminance of the background and foreground are the same, then only the color is
different and we can linearly map the coverage to alpha.

```
Ytarget = Ltarget * Ltarget
alpha   = (Ytarget - Yback) / (Yfront - Yback)       if Yfront != Yback
alpha   = coverage                                   otherwise
```

### Example

Below we calculate what we will perceptional see when anti-aliasing a vertical
line of two pixels wide centered at x=2.25. On the left side a white line
on black background and on the right side the black line on a white background.

```
           white on black-background    black on white-background
          +----+----+----+----+----+   +----+----+----+----+----+
coverage  | 0% |25% |100%|75% | 0% |   | 0% |25% |100%|75% | 0% |
          +----+----+----+----+----+   +----+----+----+----+----+
                                        ((1 - coverage)^2 - 1) / -1
          +----+----+----+----+----+   +----+----+----+----+----+
alpha     |0.0 |.062|1.0 |.562|0.0 |   |0.0 |.438|1.0 |.938|0.0 |
          +----+----+----+----+----+   +----+----+----+----+----+
                                        1 - alpha
          +----+----+----+----+----+   +----+----+----+----+----+
luminance |0.0 |.062|1.0 |.562|0.0 |   |1.0 |.562|0.0 |.062|1.0 |
          +----+----+----+----+----+   +----+----+----+----+----+
                                        sqrt(luminance)
          +----+----+----+----+----+   +----+----+----+----+----+
lightness |0.0 |0.25|1.0 |0.75|0.0 |   |1.0 |0.75|0.0 |0.25|1.0 |
          +----+----+----+----+----+   +----+----+----+----+----+
```

The perceived line width of "white on black-background" is:
`width = 0.0 + 0.25 + 1.0 + 0.75 + 0.0 = 2`

The perceived line width of "black on white-background" is:
`width = 5 - (1.0 + 0.75 + 0.0 + 0.25 + 1.0) = 2`

Sub-pixel anti-aliasing
-----------------------

During sub-pixel anti-aliasing we get a coverage value at each
sub-pixel location.

For a perceptional conversion of those coverage value to an alpha
value we are interested in the lightness of the full pixel after
compositing with the two alpha values of 0.0 and 1.0.

After that we have an alpha value for each sub-pixel, then we do
linear compositing on each sub-pixel separate.
