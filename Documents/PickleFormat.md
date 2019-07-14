# Picke encoding format



 Bit pattern                                | Description
 ------------------------------------------ | ------------------------------
 0b0VVVVVVV ... 0b1SVVVVVV                  | Little endian 2's complement positive and negative integers
 0b10VVVVVV                                 | Positive integers.
 0b110LLLLL byte ...                        | string with length.
 0xe0 - 0xf5                                | Reserved for future use.
 0xf6 value ... end-mark                    | glm::vec<>
 0xf7 binary64                              | 64 bit floating point number.
 0xf8 value ... end-mark                    | vector<>
 0xf9 (key value) ... end-mark              | map<>
 0xfa name (key value) ... end-mark         | object. Name maybe a string or integer.
 0xfb length byte ...                       | string
 0xfc                                       | false
 0xfd                                       | true
 0xfe                                       | null
 0xff                                       | end-mark




