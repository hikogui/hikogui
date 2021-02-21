Color
=====

Extended Linear sRGB
--------------------
Absolute color values uses the extended-linear-sRGB color space internally in
the application, the ttauri library and its shaders. This allows for HDR (high dynamic range)
and high gamut colors while remaining backward compatible with normal sRGB and rec709 colors.

The extended linear sRGB uses the ITU-R BT.709 color primaries and white-point, but uses
a linear transfer function, where the value (0.0, 0.0, 0.0) is 0 cd/m2 black and
(1.0, 1.0, 1.0) is 80 cd/m2 white.

Values above 1.0 for high dynamic range, and values below 0.0 for high gamut are
allowed.

Alpha
-----
Alpha is encoded as a linear floating point number between 0.0 (transparent) and 1.0 (opaque).

Inside the application and ttauri-library the color values are NOT pre-multiplied with the alpha value.
However the shaders may pre-multiply alpha internally for optimization or color accuracy reasons.

When anti-aliasing the coverage value is directly used as the alpha value during compositing.
For text we use a more accurate anti-aliasing algorithm, for details see: [Anti Aliasing](anti_aliasing.md)

Color format type
-----------------
Color format types are used in vertex arrays when communicating with the GPU.

The types have the following syntax: `numeric-type` \_ `color-components` [ \_pack ]

### Numeric type

   Type   | Description
  :------ |:-------------------------------------------------------------------------
   norm   | Signed integer mapped to the floating point range between -1.0 and 1.0.
   unorm  | Unsigned integer mapped to the floating point range between 0.0 and 1.0.
   int    | Signed integer
   uint   | Unsigned integer
   float  | Floating point number.
   srgb   | Non-linear sRGB format for the RGB component, the alpha remains linear. Values are mapped to a floating point range between 0.0 and 1.0.

### Color components

The color components are the lower case letters: r, g, b & a. The ordering of the letters describe
the order of the color components in memory. A number describes the number of bits of each component
before it.

Here are a few examples of components:

   Combination | Description
  :----------- |:---------------------
   rgba32      | 32 bits per component red, green, blue & alpha
   rgba16      | 16 bits per component red, green, blue & alpha
   rgba8       | 8 bits per component red, green, blue & alpha
   rgb8        | 8 bits per component red, green, blue
   rg8         | 8 bits per component red, green
   r8          | 8 bits per component red
   abgr8       | 8 bits per component alpha, blue, green & red
   a2bgr10     | 2 bit alpha, 10 bits per component blue, green & red

### Packing

If the format is non-packed, then each each color component must be 8, 16, 32 or 64 bits in size.
Each color component is stored in memory in native-byte-order, and the components are ordered
with in increasing memory addresses.

If the format is packed, then the color components are packed together in a single integer.
The color components are packed inside the integer from msb to lsb.
he integer is 8, 16, 32, 64 or 128 bits in size. The integer is stored in memory in native-byte-order.



