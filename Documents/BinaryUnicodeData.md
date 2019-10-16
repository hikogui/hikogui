# Binary Unicode Data Format
This is a data format that is read by the BinaryUnicodeData class and is formed from
several Unicode ucd data files.

All fields are in little endian format.

## Header

 | Type     | Name                    | Description                                                              | 
 | -------- | ----------------------- | ------------------------------------------------------------------------ |
 | uint32_t | magic                   | Identifier of this file format 'bucd' (0x62756364)                       |
 | uint32_t | version                 | Version of the file format 0x1                                           |
 | uint32_t | nrCodeUnits             | Number of code unit descriptions that follow the header                  |
 | uint32_t | nrCanonicalCompositions | Number of canonical compositions that follow the code unit descriptions  |

## Code unit description
The code unit descriptions are orded by increasing code-point value.

 | Type     | Name                    | Description                                                              | 
 | -------- | ----------------------- | ------------------------------------------------------------------------ |
 | char32_t | codePoint               | The code unit descriptions are orded by increasing code-point value      |
 | uint32_t | decompositionOffset     | Byte offset in the file where the decompositions is located.             |
 | uint8_t  | decompositionFlags      | bits 4:0 length of the decomposition 0-18. bit 5 canonical decomposition |
 | uint8_t  | decompositionOrder      | Orderering value for sorting decomposition code-points                   |
 | uint8_t  | graphemeUnitType        | bits 3:0 Type for grapheme break detecting.                              |
 | uint8_t  | reserved                | 0x0                                                                      |

## Canonical decomposition
The Canonical decompositions are ordered by increasing start code-point followed by increasing composing
code-point value.

For space saving and cache locality, the decompositionOffset of the code unit descriptions may point inside this table.

 | Type     | Name                    | Description                                                              | 
 | -------- | ----------------------- | ------------------------------------------------------------------------ |
 | char32_t | startCharacter          | The code point of the first character of a pair                          |
 | char32_t | composingCharacter      | The code point of the second character of a pair                         |
 | char32_t | composedCharacter       | The code point resulting of the combining of the pair                    |

## Grapheme unit types

 | Name                  | Value |
 | --------------------- | -----:|
 | Other                 |     0 |
 | CR                    |     1 |
 | LF                    |     2 |
 | Control               |     3 |
 | Extend                |     4 |
 | ZWJ                   |     5 |
 | Regional_Indicator    |     6 |
 | Prepend               |     7 |
 | SpacingMark           |     8 |
 | L                     |     9 |
 | V                     |    10 |
 | T                     |    11 |
 | LV                    |    12 |
 | LVT                   |    13 |
 | Extended Pictographic |    14 |