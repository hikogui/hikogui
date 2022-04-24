Color
=====

This document describes how color is used in the hikogui library.

Usage of color spaces
---------------------

 | Usage            | Space | depth   | alpha          |
 |:-----------------|:------|:--------|:---------------|
 | draw API         | tsRGB | float32 | straight       |
 | vertices         | tsRGB | float16 | straight       |
 | images           | tsRGB | float16 | pre-multiplied |
 | fragment shaders | tsRGB | float32 | pre-multiplied |
 | frame buffers    | tsRGB | float16 | pre-multiplied |
 | swap chain       | sRGB\*| uint8\* | pre-multiplied |

### PNG decoding

HikoGUI's PNG decoder will create an image of float16 RGBA values
in the tsRGB color space. The rgb values in the resulting image are
pre-multiplied by the alpha.

The PNG decoder will use the color space and transfer
function information from the PNG data to do the conversion of
pixel values to the tsRGB color space.

sRGB color space and transfer function is implied if color space
information is not available in the PNG data.

### Theme files

The colors in hikogui's theme file may be edited by users, there
are three different ways of specifying colors.

 | Format                        | Color space |
 |:------------------------------|:------------|
 | '#' 6 or 8 hex digits         | sRGB        |
 | 3 or 4 integers               | sRGB        |
 | 3 or 4 floating point numbers | tsRGB       |

### Swap chain

The swap chain images are the ones that are presented to the
desktop compositor.

On windows 10 this will be 8-bit sRGB. On Windows 10 HDR/high gamut
formats are only available for full screen applications.

On MacOS the swap chain will be in tsRGB float16, where the desktop
compositor is able to handle HDR/high gamut in the tsRGB format.

tsRGB color space
-----------------

The tsRGB color space is actually very common but never explicitly named.
Here are examples of very simular color spaces:

 - _scRGB_, specified for 16 bit biased and scaled integers
   with a range between -0.5 and 7.5. Often used outside of this spec as
   float16 or float32 with extended range.
 - _Extended-sRGB_, specified with a non-linear transfer function that is
   mirrored for negative values. Often used outside of this spec using
   a linear transfer function.
 - The unnamed color space of float16 linear RGB swap-chain images.

The tsRGB color space has the same color primaries and white-point as the
sRGB/BT.709 color spaces.

 |           | x      | y      | Luminosity contribution |
 |:----------|-------:|-------:|------------------------:|
 | Red       | 0.6400 | 0.3300 |                  0.2126 |
 | Green     | 0.3000 | 0.6000 |                  0.7152 |
 | Blue      | 0.1500 | 0.0600 |                  0.0722 |
 | White D65 | 0.3127 | 0.3290 |                         |

The RGB values have a linearly transfer function where RGB(0.0, 0.0, 0.0)
is black and RGB(1.0, 1.0, 1.0) is white with a luminance 80 cd/m2.
Values above 1.0 will produce higher luminance values available on HDR
monitors.

When all RGB values are within 0.0 and 1.0 the tsRGB color space is fully
compatible with linear sRGB.

Negative color component values will allow colors outside of the sRGB gamut
triangle, which may be rendered on high gamut monitors. However the luminosity
of the color must be greater or equal to zero.

tYUV/tLUV related color space
-----------------------------

The tLUV color space is used for mixing background and foreground
colors for anti-aliasing text, where the perceived line width is
an important attribute. For more detailed information see:
[The trouble with anti-aliasing](https://hikogui.org/2021/03/30/the-trouble-with-anti-aliasing.html)

Conversion from tRGB to tYUV/tLUV:

```
Y = R * 0.2126 + G * 0.7152 + B * 0.0722
L = sqrt(Y)
U = B - Y
V = R - Y
```

Conversion from tYUV/tLUB to tRGB:

```
Y = L * L
R = V + Y
B = U + Y
G = Y - V * 0.2973 - Y * 0.1001
```


The color types
---------------

### color

The `color` type is a 32 bit floating point 3D RGB vector + alpha scalar.

The RGB values are linear and may comprise the complete floating point range,
including values above 1.0 and value below zero.

The alpha value is a linear value between 0.0 (transparent) and 1.0 (opaque).

Depending on where the colors are used the RGB values are pre-multiplied by
the alpha value, for example: inside fragment shaders, images and frame buffers.

### matrix3

A color conversion matrix can be stored in the `matrix3` type.

A `color` value can be transformed by a `matrix3`.

The transformation ignores and preserves the alpha value.

### color storage

Color format types are used in vertex arrays and images when communicating with the GPU.

The types have the following syntax: `numeric-type` \_ `color-components` [ \_pack ]

#### Numeric type

 | Type  | Description                                                              |
 |:------|:-------------------------------------------------------------------------|
 | norm  | Signed integer mapped to the floating point range between -1.0 and 1.0.  |
 | unorm | Unsigned integer mapped to the floating point range between 0.0 and 1.0. |
 | int   | Signed integer.                                                          |
 | uint  | Unsigned integer.                                                        |
 | float | Floating point number.                                                   |
 | srgb  | Non-linear sRGB format for the RGB component, the alpha remains linear.  |

#### Color components

The color components are the lower case letters: r, g, b & a. The ordering of the letters describe
the order of the color components in memory. A number describes the number of bits of each component
before it.

Here are a few examples of components:

 | Combination | Description                                           |
 |:------------|:------------------------------------------------------|
 | rgba32      | 32 bits per component red, green, blue & alpha.       |
 | rgba16      | 16 bits per component red, green, blue & alpha.       |
 | rgba8       | 8 bits per component red, green, blue & alpha.        |
 | rgb8        | 8 bits per component red, green, blue.                |
 | rg8         | 8 bits per component red, green.                      |
 | r8          | 8 bits per component red.                             |
 | abgr8       | 8 bits per component alpha, blue, green & red.        |
 | a2bgr10     | 2 bit alpha, 10 bits per component blue, green & red  |

#### Packing

If the format is non-packed, then each color component must be 8, 16, 32 or 64 bits in size.
Each color component is stored in memory in native-byte-order, and the components are ordered
with in increasing memory addresses.

If the format is packed, then the color components are packed together in a single integer.
The color components are packed inside the integer from `msb` to `lsb`.
The integers are 8, 16, 32, 64 or 128 bits in size. The integer is stored in memory in native-byte-order.
