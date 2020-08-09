# TT5 - TTauri 5-bit string tags.

This encodes UTF-8 characters into a 64 or 128 bit integer.
It compresses the text to use as few bits possible to fit simple text.
It encodes it in such a way that the text is readable as hexspeak

## 64 bit integer.
Next 12 groups of 4 bits are used to encode the low 4 bits of each 5 bit code from the table below.
Next 12 bits are used to encode the high but of each 5 bit code from the table below.

Next two bits encode:
 * 0 - Start string in P0
 * 1 - Start string in P1
 * 2 - Start string in P1, then switch to P0
 * 3 - Start string in P1, then switch to P0. Do this after every separator.

Next bit whitespace 0x1b:
 * 0 - Underscore
 * 1 - Space


## Table

 | #  | P0  | P1  | P2  |
 | --:|:--- |:--- |:--- |
 | 00 | o   | O   | 0   |
 | 01 | i   | I   | 1   |
 | 02 | z   | Z   | 2   |
 | 03 | w   | W   | 3   |
 | 04 | h   | H   | 4   |
 | 05 | s   | S   | 5   |
 | 06 | y   | Y   | 6   |
 | 07 | t   | T   | 7   |
 | 08 | x   | X   | 8   |
 | 09 | g   | G   | 9   |
 | 0a | a   | A   |     |
 | 0b | b   | B   |     |
 | 0c | c   | C   |     |
 | 0d | d   | D   |     |
 | 0e | e   | E   |     |
 | 0f | f   | F   |     |
 | 10 | u   | U   |     |
 | 11 | l   | L   |     |
 | 12 | r   | R   |     |
 | 13 | m   | M   |     |
 | 14 | p   | P   |     |
 | 15 | n   | N   |     |
 | 16 | v   | V   |     |
 | 17 | j   | J   |     |
 | 18 | k   | K   |     |
 | 19 | q   | Q   |     |
 | 1a | NUL | NUL | NUL |
 | 1b | _   | _   | _   |
 | 1c | .   | .   | .   |
 | 1d | -   | -   | -   |
 | 1e | S1  | S0  | S0  |
 | 1f | S2  | S2  | S1  |

