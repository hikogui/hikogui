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

### Values
The types for values are the same as those of JSON.

 * String
 * Boolean
 * Null
 * Number
 * Array
 * Object

```
value := string | boolean | null | number | array | object
```

### String
Strings are valid unicode code points encoded as UTF-8.
The end-of-string is indicated by the start of new value or the
end of an object, array or end-of-file

Consecutive strings start with 0xf8 to denote the end of the previous
string.

```
string := 0xf8? utf8*

utf8 := 
      0x00-0x7f
    | 0xc0-0xdf 0x80-0xbf
    | 0xe0-0xef 0x80-0xbf 0x80-0xbf
    | 0xf0-0xf7 0x80-0xbf 0x80-0xbf 0x80-0xbf
```

### Number
JSON does not differientiate between integer or floating point,
however most implementations will treat numbers with a decimal
point as a floating point number, otherwise as integer.

#### Integer
Positive integers between 0 and 31 are encoded as a single
byte using the following expression `value + 0x80`.

Negative integers between -1 and -24 are encoded as a single
byte using the following expression `abs(value) - 1 + 0xa0`

Other integer values are encoded as a signed integer in big
endian order in 1, 2, 4 or 8 bytes.

```
integer := 
    0x80-0x9f
    | 0xa0-0xb7
    | 0xb8 byte
    | 0xb9 byte{2}
    | 0xba byte{4}
    | 0xbb byte{8}
```

#### Floating point
Floating point numbers are encoded as IEEE-754 binary32 or
binary64 in big endian byte order.

```
float := 0xbc byte{4} | 0xbd byte{8}
```

### Boolean
False is encoded as 0xbe, True is encoded as 0xbf.

```
boolean := 0xbe | 0xbf
```

### Null
Null is encoded as 0xf9.

```
null := 0xf9
```

### Array
An empty array is encoded as 0xfa. Other arrays are encoded with 0xfb
followed by consecutive values and ending in 0xfc.

```
array := 0xfa | 0xfb value* 0xfc
```

### Object
An empty object is encoded as 0xfd. Other objects are encoded with 0xfe
followed by consecutive key-values pairs and ending in 0xff. The
keys are always strings.

```
object := 0xfd | 0xfe ( string value )* 0xff
```

## Canonical
Canonical FBON is used to encode data that needs to be cryptographically
signed.

The following rules must be followed to make canonical FBON:
 * String MUST ONLY start with 0xf8 if it directly follows another string.
 * Integers MUST be encoded in the least amount of bytes
 * Floating point numbers MUST be encoded in the least amount of bytes
   while preserving precision.
 * Object keys MUST be lexically ordered based on the UTF-8 code units.
 * Empty arrays MUST be encoded using 0xfa
 * Empty objects MUST be encoded using 0xfd


