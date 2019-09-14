# License Format
This document describes the format of a license-code that a customer
can enter in the application to unlock functionality.

## Base-93 SECDEC31:26
This encoding has some interesting properties for use with long
license code.

 * Base-93 uses the printable ASCII characters from '!' up to and
   including '}'.
 * The '~' character is used as a command character.
 * All non-printable ASCII characters are ignored.
 * A base-93-group: each character is treated as a
   base-93 digit in big endian notation.
 * Every base-93-group is 13 characters in length,
   except the last one in a message.
 * The last base-93-group of a message is truncated
   to only encode the bytes needed. The number of base-93 digits
   will determine the number of full bytes encoded, acording to
   the following list (excluding base-93-secdec):

     * 2 characters -> 13 bits -> 8 data + 5 checksum
     * 3 characters -> 19 bits -> 16 data + 3 checksum 
     * 4 characters -> 26 bits -> 24 data + 2 checksum
     * 6 characters -> 39 bits -> 32 data + 7 checksum
     * 7 characters -> 45 bits -> 40 data + 5 checksum
     * 8 characters -> 52 bits -> 48 data + 4 checksum
     * 9 characters -> 58 bits -> 56 data + 2 checksum
     * 11 characters -> 71 bits -> 64 data + 7 checksum
     * 12 characters -> 78 bits -> 72 data + 6 checksum
     * 13 characters -> 85 bits -> 80 data + 5 checksum

 * The parity check is calculated by doing a modulo-93 sum
   of all characters, including the parity; yielding zero.

## Encoding alphabet.
The alphabet of the encoding is designed as follows:

 * Useful for base-85 encoding (5 characters -> 32 bit integer)
 * All characters are printable basic-latin-1
 * Characters with simular glyphs 0,O,o and 1,I,l get the same code point
 * A letter's upper and lower case form are mapped to code points with
   different parity.
 * Don't include quotes '"' so it can be copy pasted in literal strings
   in most programming languages.
 * Don't use backslash '\' because it is used as an escape character in
   most programming languages.
 * Don't use minus '-' or underscore '\_' so it can be used to seperate groups.


```
 0 0Oo    1 1lI    2 2      3 3      4 4      5 5     6 6     7 7 
 8 8      9 9     10 a     11 b     12 c     13 d    14 e    15 f 
16 g     17 h     18 i     19 j     20 k     21 .    22 m    23 n 
24 ,     25 p     26 q     27 r     28 s     29 t    30 u    31 v 
32 w     33 x     34 y     35 z     36 @     37 A    38 B    39 C 
40 D     41 E     42 F     43 G     44 H     45 :    46 J    47 K 
48 L     49 M     50 N     51 ;     52 P     53 Q    54 R    55 S 
56 T     57 U     58 V     59 W     60 X     61 Y    62 Z    63 ! 
64 ?     65 #     66 $     67 %     68 ^     69 &    70 *    71 + 
72 =     73 (     74 )     75 {     76 }     77 [    78 ]    79 < 
80 >     81 ?     82 !     83 /     84 | 
```

## Use Base 58

Like base-64 but using only letters and numbers and merging
problematic readable characters.

```
 0 0O     1 1lI    2 2      3 3      4 4      5 5S    6 6     7 7 
 8 8      9 9     10 a     11 b     12 c     13 d    14 e    15 f 
16 g     17 h     18 i     19 j     20 k     21 m    22 n    23 o 
24 p     25 q     26 r     27 s     28 t     29 u    30 v    31 w 
32 x     33 y     34 z     35 A     36 B     37 C    38 D    39 E 
40 F     41 G     42 H     43 J     44 K     45 L    46 M    47 N 
48 P     49 Q     50 R     51 T     52 U     53 V    54 W    55 X 
56 Y     57 Z 
```

```
 0 a   1 b   2 c   3 d  4 e  5 f  6 g  7  
 8 h   9 i  10 j  11 k 12 l 13 m 14 n 15  
16 o  17 p  18 q  19 r 20 s 21 t 22   23  
24    25    26    27   28   29   30   31  
32    33    34    35   36   37   38   39  
40    41    42    43   44   45   46   47  
48    49    50    51   52   53   54   55  
56    57 

```
## Error check & correction
Using hamming-code instead of working with bits and parities we work
with characters and weighted-checksums. We get a 11 data characters
plus 4 checksum character plus an aditional checksum character. These
16 characters are enough to encode a single 64 bit word.

```

base-58 (2 x 64bits => 16 bytes/28char=0.57)
ppDpDDDpDDDDDDD-pDDDDDDDDDDD-....P
123456789abcdef-0123456789ab-cdef0

base-58 (2 x 64bits => 16 bytes/32char=0.50)
ppDpDDDpDDDDDDDP/ppDpDDDpDDDDDDDP
123456789abcdef0/123456789abcdef0

base-58 (1 x 64bits => 8 bytes/15char=0.53)
ppDpDDDpDDDDDDD
123456789abcdef

base-64 (6 x 24bits => 18 bytes/30char=0.6)
ppDpDDD-pDDDD-DDDpD-DDDD-DDDD-DDDD-..P
1234567-89abc-def01-2345-6789-abcd-ef0

base-64 (4 x 24bits => 12 bytes/32char=0.375)
ppDpDDDP/ppDpDDDP/ppDpDDDP/ppDpDDDP
12345670/12345670/12345670/12345670

base-64 (4 x 24bits => 12 bytes/28char=0.429)
ppDpDDD/ppDpDDD/ppDpDDD/ppDpDDD
1234567/1234567/1234567/1234567

base-85 (5 x 32bits => 20 bytes/31char=0.64)
ppDpDDDpD-DDDDD-DpDDDD-DDDDD-DDDDD-.P
123456789-abcde-f01234-56789-abcde-.0

base-94 (72bits -> 11 characters) (all printable chars)

# Very high encoding with short strings. Uses all printable
# ASCII characters except one, maybe don't include backslash.
base-93 chars 13 bits 85 ratio 6.538462
base-93 (680 bits => 85bytes/ bytes 0.664)
Use hamming 31:26 + extra parity, we encode 170 bits in 32 characters.
Use hamming truncated 127:120 + extra parity, 85 bytes as 104 characters + 8 parity characters.

```

## 32-bit group


A 32-bit group is encoded as 7 characters from the encoding alphabet.

```
           p0 (lsb)    p1       p2       p3       p4    p5 (msb)
 +--------+--------+--------+--------+--------+--------+--------+
 | parity |  sum   | code-0 | code-1 | code-2 | code-3 | code-4 |
 +--------+--------+--------+--------+--------+--------+--------+
```

The parity character has a value between 0-64 representing 6 bits of parity
the bits are assigned to each of the next 6 characters; least to
most-significant-bit. Parity bit is set when the code point of the
corrosponding character is even.

The sum-character is the modulo-85 sum of the parity and 5 code characters. It
can be used to recover a single character error (including the sum
itself). It can also be used to detect errors when the parity character
does not.

The 32-bit value is constructed from the base-85 code characters as follows:

```
uint32_t value = 0;
for (int i = 0; i < 5; i++) {
    value *= 85;
    value += code[4 - i];    
}
```

## License code

```
 +--------+--------+--------+--------+--------+--------+--------+--------+--------+
 |               Name                | feature| until  | serial |    signature    |
 +--------+--------+--------+--------+--------+--------+--------+--------+--------+
```

 * Name - 16 byte UTF-8 customer's name
 * Until - Julian date
 * Serial - Unique serial number for this license
 * Signature - Lower 64-bit of the md5sum on the license code concatenated with
   the application secret.


