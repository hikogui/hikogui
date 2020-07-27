# Fast Binary Object Notation

Here are the reasons for creating the FBON data format:
 * Exact translation between JSON and FBON
 * A canonical representation to allow signing of messages.
 * No extensibility, allows every parser to handle every FBON.
 * Low amount of overhead
 * Quick encode / decode.

Here are some other object-notation formats looked at:
 * Message Pack (complicated to parse, more data types than JSON)
 * BSON (data overhead, more data types than JSON)
 * CBOR (complicated concatonated data structure, more data types than JSON)
 * Smile (back references complicate encoding and decoding)
 * UBJSON (more data overhead)

## Encodig
The code-unit 0x80-0xbf and 0xf8-0xff are outside the valid range
of valid UTF-8 start-code-units. These codes can be used as start
codes for other types of values.

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

array := empty_array | start_array value* eoc;

object := empty_object | start_object (string value)* eoc
```

  Code Sequence                                | # | Type                      | Bits |      Min |       Max
 :----------------------------------------     | -:|:------------------------- | ----:| --------:| ----------:
  0x00 - 0x7f                                  | 1 | UTF-8 One byte sequence   |    7 |   U+0000 |    U+007f
  0xc2-0xdf 0x80-0xbf                          | 2 | UTF-8 Two byte sequence   |   11 |   U+0080 |    U+07ff      
  0xe0-0xef 0x80-0xbf 0x80-0xbf                | 3 | UTF-8 Three byte sequence |   16 |   U+0800 |    U+ffff
  0xf0-0xf7 0x80-0xbf 0x80-0xbf 0x80-0xbf      | 4 | UTF-8 Four byte sequence  |   21 | U+100000 |  U+10ffff
  0x80 - 0xaf                                  | 1 | Positive Integer          |      |        0 |        47
  0xb0 - 0xb9                                  | 1 | Negative Integer          |      |       -1 |       -10
  0xba                                         | 1 | Floating point -1.0       |      |          |
  0xbb                                         | 1 | Floating point 0.0        |      |          |
  0xbc                                         | 1 | Floating point 1.0        |      |          |
  0xbd                                         | 1 | Empty Array               |      |          |
  0xbe                                         | 1 | Empty Object              |      |          |
  0xbf                                         | 1 | Null                      |      |          |
  0xc0                                         | 1 | False                     |      |          |
  0xc1                                         | 1 | True                      |      |          |
  0xc2-0xdf 0x00-0x7f                          | 2 | Positive Integer (30*128) |      |        0 |      3839
  0xe0-0xef 0x00-0x7f byte                     | 3 | Positive Integer          |   19 |        0 |    524287
  0xf0-0xf7 0x00-0x7f byte byte                | 4 | Positive Integer          |   26 |        0 |  67108863
  0xc2-0xdf 0xc0-0xff                          | 2 | Negative Integer (30*64)  |      |       -1 |     -1920
  0xe0-0xef 0xc0-0xff byte                     | 3 | Negative Integer          |   18 |       -1 |   -262144
  0xf0-0xf7 0xc0-0xff byte byte                | 4 | Negative Integer          |   25 |       -1 | -33554432
  0xf8 byte byte byte byte                     | 5 | Signed integer            |   32 |   -2**31 | 2**31 - 1
  0xf9 byte byte byte byte byte byte byte byte | 9 | Signed integer            |   64 |   -2**63 | 2**63 - 1
  0xfa byte byte byte byte                     | 5 | Floating point            |   32 |          | 
  0xfb byte byte byte byte byte byte byte byte | 9 | Floating point            |   64 |          | 
  0xfd                                         | 1 | Start Array               |      |          |
  0xff                                         | 1 | Start Object              |      |          |
  0xfe                                         | 1 | End Of Container (eoc)    |      |          |
  0xff                                         | 1 | End Of Text (eot)         |      |          |
                                               |   |                           |      |          |
  0xe0-0xef 0x80-0xbf 0x00-0x7f                | 3 | Invalid                   |      |          |
  0xe0-0xef 0x80-0xbf 0xc0-0xff                | 3 | Invalid                   |      |          |
  0xf0-0xf7 0x80-0xbf 0x00-0x7f byte           | 4 | Invalid                   |      |          |
  0xf0-0xf7 0x80-0xbf 0x80-0xbf 0x00-0x7f      | 4 | Invalid                   |      |          |
  0xf0-0xf7 0x80-0xbf 0x80-0xbf 0xc0-0xff      | 4 | Invalid                   |      |          |
  0xf0-0xf7 0x80-0xbf 0xc0-0xff byte           | 4 | Invalid                   |      |          |
























## Extra rules
The rules below ensures minimum message size and allows for cryptographically
signing of the message in a repeatable way. All encoders MUST follow these
rules.

 * String MUST ONLY end with 0xff if at least one of the following is true:
   - The string is empty,
   - When another string is directly following this string,
   - If the full message is this string.
 * Integers MUST be encoded in the least amount of bytes.
 * Floating point numbers MUST be encoded in the least amount of bytes.
   while preserving precision. In other words: a binary64 number needs to be converted to
   binary32 and back to determine if it can be encoded as binary32 while preserving precision.
 * Object keys MUST be lexically ordered based on encoded bytes.


