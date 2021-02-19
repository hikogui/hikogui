
The color space of a fragment shader
------------------------------------
The fragment shader does color calculations in floating point, the input and output are
both in the sRGB color space with a linear transfer function.
Where the values 0.0 to 1.0 are mapped-linearly to 0 cd/m2 to 80 cd/m2.

The range of black to 80 cd/m2 is the range of a correctly calibrated HDR monitor displaying
the normal range of the sRGB color space. 

Preceived brightness by the human eye 
-------------------------------------
But although the linear colors values in the fragment shader will be represented
linearly by the amount of light coming from the monitor, the human eye preceived
this amount of light logarithmically.

Reference: <https://en.wikipedia.org/wiki/Lightness>

CIELAB is a color space and transfer function for preceived uniform color and brightness
by the human eye.

CIELAB specifies uniform lightness as:
```
L = L\* / 100.0

L\* = 116 * f(Y / Yn) - 16

f(t) = cbrt(t)                          if t > (6/29)^3
       1/3 * (29/6)^2 * t + (4 / 29)    otherwise

Yn = 80 cd/m2, Y = light from monitor

or from the point of view of the fragment shader:

Yn = 1.0, Y = color value from the shader
```

This is a pretty complicated to calculate inside the fragment shader
so as a shortcut the estimate from 1920 is a simple:

```
L = sqrt(Y)

Y = color value from the shader
```

Linear coverage to alpha compositing
------------------------------------
As everyone knows alpha compositing of two images must be done with
linear colour values or there will be color and brightness changes
where the alpha value is not either 0.0 or 1.0.

If however we linearly convert the pixel coverage of an antialiased line
to an alpha value between 0.0 and 1.0 we get the issue that the preceived
width of the line is different between light-on-dark or dark-on-light.

Below we calculate what we will perceptionally see when anti-aliasing a vertical
line of two pixels wide, offset by 1/3 of a pixel. On the left side a white line
on black background and on the right side the white line on a black background.

```
coverage:
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
 |  0% | 33% |100% | 66% |  0% |         |  0% | 33% |100% | 66% |  0% |
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+

alpha:
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
 | 0.0 |0.333| 1.0 |0.666| 0.0 |         | 1.0 |0.666| 0.0 |0.333| 1.0 |
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+

linear pixel value after compositing:
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
 | 0.0 |0.333| 1.0 |0.666| 0.0 |         | 1.0 |0.666| 0.0 |0.333| 1.0 |
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+

preceived lightness:
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
 | 0.0 |0.574| 1.0 |0.812| 0.0 |         | 1.0 |0.812| 0.0 |0.574| 1.0 |
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
```

Now we can calculate the line width based on the pixel values, simply
sum the values of the pixels.
We need to invert (1.0 - x) the values for black-on-white line for this calculation.

```
invert black on white line lightness, for calculating linear line width:
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
 | 0.0 |0.333| 1.0 |0.666| 0.0 |         | 0.0 |0.333| 1.0 |0.666| 0.0 |
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
  width = 2.0                             width = 2.0

invert black on white line lightness, for calculating preceived line width:
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
 | 0.0 |0.574| 1.0 |0.812| 0.0 |         | 0.0 |0.188| 1.0 |0.426| 0.0 |
 +-----+-----+-----+-----+-----+         +-----+-----+-----+-----+-----+
  width = 2.386                           width = 1.614
```

As you see when you do linear compositing the perceived line width of a black line
on white background is wider than a white line on black background.

Perceived coverage to alpha compositing
---------------------------------------
Since we are correctly using linear compositing colors with the alpha
value. There can be only one conclusion: it is not correct to convert
a coverage linearly to an alpha value.

Since with coverage value we want to determine the lightness (perceptional)
of a pixel based on a mix of the background and foreground color. We
first need to get the lightness of a color.

The following is the formula to covert sRGB color space to CIE luminance (Y),
onverted to 1920 lightness:

```
L = sqrt(Y)
Y = 0.2126 * R + 0.7152 * G + 0.0722 * B
```

The wanted lightness and luminance based on coverage is:

```
Lwanted = mix(Lback, Lfront, coverage), 
Ywanted = Lwanted * Lwanted
```

To get the alpha value we need to find the relative distance between the luminances
of Ywanted between Yback and Yfront.

```
alpha = (Ywanted - Yback) / (Yfront - Yback)       if Yfront != Yback
alpha = coverage                                   otherwise
```

Perceived coverage to alpha compositing with subpixel anti-aliasing
-------------------------------------------------------------------
During subpixel anti-aliasing we get a coverage value at each
sub-pixel location. Since from the previous chapter we determined
that we are interested in the lightness of a color.

So we convert the sub-pixel coverage to sub-pixel alpha based
on the overal lightness of the full pixel, not just on a component.

After that we have a sub-pixel alpha, then we do linear compositing
on each sub-pixel separate.

Alpha included in the foregound color or background color
---------------------------------------------------------

