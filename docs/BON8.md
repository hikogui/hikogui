Binary Object Notation (BON8)
=============================

Here are the reasons for creating the BON8 data format:

 - Exact translation between JSON and BON8
 - A canonical representation to allow signing of messages.
 - No extensibility, allows every parser to handle every BON8.
 - Low amount of overhead
 - Quick encode / decode.

Here are some other object-notation formats looked at:

 - Message Pack (complicated to parse, more data types than JSON)
 - BSON (data overhead, more data types than JSON)
 - CBOR (complicated concatenated data structure, more data types than JSON)
 - Smile (back references complicate encoding and decoding)
 - UBJSON (more data overhead)

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

array := start_array_with_count value* | start_array value* eoc;

object := start_object_with_count (string value)* | start_object (string value)* eoc
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

The rules below ensures minimum message size and allows for cryptographically
signing of the message repeatably. All encoders MUST follow these rules.

 - String MUST ONLY end with 0xff if at least one of the following is true:
   - The string is empty,
   - When another string is directly following this string,
   - If the string ends the message.
 - Integers MUST be encoded in the least amount of bytes.
 - Floating point numbers MUST be encoded in the least amount of bytes
   while preserving precision. In other words: a binary64 number needs to be converted to
   binary32 and back to determine if it can be encoded as binary32 while preserving precision.
 - Object keys MUST be lexically ordered based on UTF-8 code units.

