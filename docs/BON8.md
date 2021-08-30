Binary Object Notation (BON8)
=============================

Here are the reasons for creating the BON8 data format:

 - Exact translation between JSON and BON8
 - A canonical representation to allow signing of messages.
 - No extensibility, allows every parser to handle every BON8.
 - Low amount of overhead.
 - Quick encode / decode.
 - Self terminating, a valid BON8 message can be decoded without a buffer-length.

Encoding
--------

The idea of this encoding is that strings are encoded as UTF-8.

UTF-8 has a lot of codes that fall within the invalid range which
can be used to encode different kinds of data.

All of these invalid UTF-8 start-code-units have been assigned below
to reduce the ability to extent the format, and to improve compression
of the data.

```
message := value;

value := number | boolean | null | string | array | object;

number := integer | float;

integer := positive_integer | negative_integer | signed_integer;

float := binary32 | binary64;

boolean := true | false;

string := character character* | character* eot;

character := utf8_1 | utf8_2 | utf8_3 | utf8_4;

array := start_array_with_count value*
       | start_array value* eoc;

object := start_object_with_count (string value)*
        | start_object (string value)* eoc
```

This table gives an overview on the encoding:

  Code Sequence           | # | Type                        | Bits |      Min |       Max
 :----------------------- | -:|:-------------------------   | ----:| --------:| ----------:
  00-7f                   | 1 | UTF-8 One byte sequence     |    7 |   U+0000 |    U+007f
  c2-df 80-bf             | 2 | UTF-8 Two byte sequence     |   11 |   U+0080 |    U+07ff
  e0-ef 80-bf byte\*1     | 3 | UTF-8 Three byte sequence   |   16 |   U+0800 |    U+ffff
  f0-f7 80-bf byte\*2     | 4 | UTF-8 Four byte sequence    |   21 | U+100000 |  U+10ffff
  80-84                   |   | Start array with count      |      |        0 |         4
  85                      |   | Start array                 |      |          |
  86-8a                   |   | Start object with count     |      |        0 |         4
  8b                      |   | Start object                |      |          |
  8c byte\*4              | 5 | Signed integer              |   32 |    -2^31 |  2^31 - 1
  8d byte\*8              | 9 | Signed integer              |   64 |    -2^63 |  2^63 - 1
  8e byte\*4              | 5 | Floating point binary32     |   32 |          |
  8f byte\*8              | 9 | Floating point binary64     |   64 |          |
  90-b7                   | 1 | Positive integer            |      |        0 |        39
  b8-c1                   | 1 | Negative Integer            |      |       -1 |       -10
  c2-df 00-7f             | 2 | Positive Integer (30 * 128) |      |        0 |      3839
  e0-ef 00-7f byte\*1     | 3 | Positive Integer            |   19 |        0 |    524287
  f0-f7 00-7f byte\*2     | 4 | Positive Integer            |   26 |        0 |  67108863
  c2-df c0-ff             | 2 | Negative Integer (30 * 64)  |      |       -1 |     -1920
  e0-ef c0-ff byte\*1     | 3 | Negative Integer            |   18 |       -1 |   -262144
  f0-f7 c0-ff byte\*2     | 4 | Negative Integer            |   25 |       -1 | -33554432
  f8                      | 1 | False                       |      |          |
  f9                      | 1 | True                        |      |          |
  fa                      | 1 | null                        |      |          |
  fb                      | 1 | -1.0                        |      |          |
  fc                      | 1 | 0.0                         |      |          |
  fd                      | 1 | 1.0                         |      |          |
  fe                      | 1 | End Of Container (eoc)      |      |          |
  ff                      | 1 | End Of Text (eot)           |      |          |


Extra rules
-----------

The rules below ensures minimum message size and consistency, which allows for cryptographically
signing of messages. All encoders MUST follow these rules.

 - A message is a single value. Most often this value is of type Object or type Array.
 - String MUST ONLY end with eot (0xff) if at least one of the following is true:
   - The string is empty,
   - When another string is directly following this string,
   - If the string ends the message.
 - Strings MUST be a valid UTF-8 encoded string.
 - Strings MAY contain any Unicode code-point between U+0000 and U+10ffff.
 - Unicode code-points MUST be encoded with the least amount of UTF-8 code-units.
 - Integers MUST be encoded in the least amount of bytes.
 - Floating point numbers MUST be encoded in the least amount of bytes
   while preserving precision. After conversion of a binary64 to binary32
   the resulting value must be within 1 ULP of the original value,
   this will allow for different rounding directions between systems.
   Below is an C++ example for determining if a binary64 number can be
   represented as binary32.
 - Arrays MUST be encoded with the least amount of bytes.
 - Objects MUST be encoded with the least amount of bytes.
 - The keys of an Object MUST be lexically ordered based on UTF-8 code-units.

```cpp
bool binary64_can_be_represented_as_binary32(double x)
{
    if (std::isnan(x) or std::isinf(x)) {
        return true;
    } else if (x < -std::numeric_limits<float>::max() or x > std::numeric_limits<float>::max) {
        return false;
    } else {
        auto y = static_cast<float>(x);
        auto a = std::nextafter(x, -std::numeric_limits<double>::infinity());
        auto b = std::nextafter(x, std::numeric_limits<double>::infinity());
        return a <= y and y <= b;
    }
}
```