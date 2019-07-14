# Picke encoding format



 Bit pattern                                | Description
 ------------------------------------------ | ------------------------------
 0b0VVVVVVV ... 0b1SVVVVVV                  | Little endian 2's complement positive and negative integers
 0b10VVVVVV                                 | Positive integers.
 0b110LLLLL byte ...                        | string with length.
 0b1110XXXX                                 |
 0b111100XX                                 |
 0b11110100                                 |
 0b11110101                                 |
 0b11110110 value ... end-mark              | glm::vec<>
 0b11110111 binary64                        | 64 bit floating point number.
 0b11111000 value ... end-mark              | vector<>
 0b11111011 (key value) ... end-mark        | map<>
 0b11111010 name (key value) ... end-mark   | object. Name maybe a string or integer.
 0b11111011 length byte ...                 | string
 0b11111100                                 | false
 0b11111101                                 | true
 0b11111110                                 | null
 0b11111111                                 | end-mark




