# Picke encoding format



## Integer
Any integer is encoded as a variable length big endian 2's compliment signed integer in groups of 7 bits. Bit-7 is used
to encode a continue-bit. Bit-6 of the first group is the sign bit, according to the 2's compliment encoding. Due to stop
bit encoding an integer is encoded by at least two groups.

 0b1SVVVVVV 0b1VVVVVVV ... 0b0VVVVVVV

A positive integer between 0x00 - 0x20 can be directly send to the output.


## Strings
### ASCII String
If a text string:
 * Has two or more characters.
 * The first character is a value between 0x60-0x7e
 * The rest of the characters is a value between 0x00-0x7f (including non-terminating zero values)

Then the string can be directly send to the output, with the last character's bit-7 set high, to form a stop-bit.

 0b0CCCCCCC 0b0CCCCCCC ... 0b1CCCCCCC

### UTF String
If a text string:
 * Has zero or more characters
 * Does not contain a zero (0x00) value.

Then the string can be directly send to the output after a 0x2f byte and ending with a 0x00 byte.

 0x2f 0bCCCCCCCC 0bCCCCCCCC ... 0x00

### Binary string
If a binary string:
 * has zero or more characters
 * Contains zero or more zero (0x00) values

Then the string can be directly send to the output after a 0x2e byte and by an Integer
for the size.

 0x2e Integer-size 0bCCCCCCCC 0bCCCCCCCC ...

## Simple values
### Null
 0x2d

### True
 0x2c

### False
 0x2b

## Vector
A vector starts with a 0x2a byte, followed by encoded values, terminated by a 0x7f byte.

 0x2a value value ... 0x7f

## Dictionary
A dictionary starts with a 0x29 byte, followed by encoded key-value pairs, terminated by
a 0x7f byte.

 0x29 key value key value ... 0x7f

## Floating point numbers
A floating point number is encoded as a 64-bit floating point number in little-endian
byte order, prefixed with a 0x28 byte.

 0x28 0

## Struct/Class
Arbitrary structs can be encoded with the 0x27 byte followed by the name of the struct
followed by values representing the members in that struct and ending in 0x7f.

For well known name we will use integers. For custom structs string-names should be used.

 0x27 String-name value value ... 0x7f
 0x27 Integer-name value value ... 0x7f





