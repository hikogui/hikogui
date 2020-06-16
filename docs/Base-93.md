# Base-93 format
The base-93 format is used to encode binary data into ASCII data
so that a user is able to copy this data using copy-and-paste from
one application to another.

This is for example useful for license code, or for moving snippets
of application settings.

The encoding efficiency is calculated as follows:
 * 21 bytes are encoded as 26 base-93 digits
 * The 26 digits are encapsulated inside SECDEC as 32 characters
 * 2 synchronization characters are appended giving 34 characters
 * Ratio is 21 Bytes / 34 chars => 0.62 bytes/character
 * Ratio is 168 Bits / 34 chars => 4.94 bits/character

## Alphabet
The base-93 alphabet contains all 94 printable ASCII characters.

The first 93 characters ASCII '!' up to and including '}' represents
a base-93 digit. '!' codes the value 0, '}' encodes the value 92.

ASCII '~' character is used to start a command.

All non-printable characters are ignored, characters outside the
ASCII range are an error.

## Commands

### Start data
The characters "~b93{" starts a data section. Character left of "~b93{"
are ignored by the decoder, this allows a clear-text description
of the data.

### End data
The characters "~}" stops a data section. Character right of "~}"
are ignored by the decoder, this allows a clear-text description
of the data.

## Base-93 Number
A base-93 number is 13 characters in length, except for the
last number in a message which may be smaller depending
on the number of bytes encoded, as shown in the table below:

| digits | bits   | bytes  | unused |
| ------:| ------:| ------:| ------:|
|      2 |     13 |      1 |      0 |
|      4 |     26 |      2 |      5 |
|      5 |     32 |      3 |      3 |
|      6 |     39 |      4 |      2 |
|      7 |     45 |      5 |      0 |
|      9 |     58 |      6 |      5 |
|     10 |     65 |      7 |      4 |
|     11 |     71 |      8 |      2 |
|     12 |     78 |      9 |      1 |
|     13 |     85 |     10 |      0 |

Constructing a 85 bit unsigned integer from these digits as follows:
 1. For each character left-to-right
 2. multiple the integer with 93, then
 3. add the base-93 value of the character to that integer.

The number includes a number of bytes, a 5 bit CRC and unused bits
which are set to zero.

We use as basis the CRC-5-USB polynomial `x^5 + x^2 + 1` which yields a divisor 0b1'00101.
Each number of 13 digits and smaller is first converted to an 85 bit unsigned integer and then
the location of this number is prepended in front of this integer. The location starts at
zero and for each number in the message it is incremented by one.

The checksum is correct when the result is zero in the algorithm below:
```
uint5_t crc5_base93(uint85_t decoded_number, uint32_t location) {
    auto result = location << 85 | decoded_number;
    auto divisor = 0b100101 << 111;

    for (auto i = 0; i < 111; i++) {
        result ^= divisor;
        divisor >>= 1;
    }
    return result;
}
```

The bytes are extracted from the number with the following algorithm:
```
auto extract_bytes_base93(uint85_t decoded_number, int nr_bytes) {
    std::string result;

    decoded_number >>= 5;
    for (auto i = 0; i < nr_bytes; i++) {
        result += static_cast<char>(decoded_number & 0xff);
        decoded_number >>= 8;
    }
    return result;
}
```

