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

## Table
The following is a table of how each type is encoded.

 | First   | Next                 | Type             |
 +---------+----------------------+----------------  +---------------
 | 00 - 7f |                      | Character        |
 | 80 - af |                      | Integer 0 - 47   | `Value + 0x80`
 | b0 - ba |                      | Integer -1 - -11 | `abs(Value) - 1 + 0xb0`
 | bb      |                      | False            |
 | bc      |                      | True             |
 | bd      |                      | Null             |
 | be      |                      | eoc              |
 | bf      |                      | eot              |
 | c0 - df | Byte                 | Character        |
 | e0 - ef | Byte{2}              | Character        |
 | f0 - f7 | Byte{3}              | Character        |
 | f8      | Byte                 | int8             | signed integer
 | f9      | Byte{2}              | int16            | big endian signed integer
 | fa      | Byte{4}              | int32            | big endian signed integer
 | fb      | Byte{8}              | int64            | big endian signed integer
 | fc      | Byte{4}              | binary32         | big endian IEEE-754 binary32
 | fd      | Byte{8}              | binary64         | big endian IEEE-754 binary64
 | fe      | Value\* eoc          | Array            |
 | ff      | (String Value)\* eoc | Object           |

## BNF
The following is an extended BNF description of the protocol.

```
message := value;
value := object | array | string | number | boolean | null;
number := integer | float;
integer := pos_short_int | neg_short_int | int8 | int16 | int32 | int64
float := binary32 | binary64;
boolean := true | false;
key_value := string value;
string := character character* | character* eot;
character := ascii | utf8_2 | utf8_3 | utf8_4;

ascii := 0x00 - 0x7f;
pos_short_int := 0x80 - 0xaf;
neg_short_int := 0xb0 - 0xba;
false := 0xbb;
true := 0xbc;
null := 0xbd;
eoc := 0xbe;
eot := 0xbf;
utf8_2 := 0xc0-0xdf utf8cont;
utf8_3 := 0xe0-0xef utf8cont utf8cont;
utf8_4 := 0xf0-0xf7 utf8cont utf8cont utf8cont;
int8 := 0xf8 byte;
int16 := 0xf9 byte byte;
int32 := 0xfa byte byte byte byte;
int64 := 0xfb byte byte byte byte byte byte byte byte;
binary32 := 0xfc byte byte byte byte;
binary64 := 0xfd byte byte byte byte byte byte byte byte;
array := 0xfe value* eoc;
object := 0xff key_value* eoc;

byte := 0x00-0xff;
utf8cont := 0x80-0xbf;
```

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


