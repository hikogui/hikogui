# TT5 A 5 bit code designed for encoding identifiers.

```
 | #  | P0  | P1  | P2   |
 | --:|:--- |:--- |:---- |
 | 00 | '\0'| '\0'| '\0' |
 | 01 | a   | A   | '\t' |
 | 02 | b   | B   | U2   |
 | 03 | c   | C   | U3   |
 | 04 | d   | D   | U4   |
 | 05 | e   | E   | U5   |
 | 06 | f   | F   | '\n' |
 | 07 | g   | G   | ' '  |
 | 08 | h   | H   | A0   |
 | 09 | i   | I   | A1   |
 | 0a | j   | J   | A2   |
 | 0b | k   | K   | A3   |
 | 0c | l   | L   | ,    |
 | 0d | m   | M   | ;    |
 | 0e | n   | M   | @    |
 | 0f | o   | O   | /    |
 | 10 | p   | P   | 0    |
 | 11 | q   | Q   | 1    |
 | 12 | r   | R   | 2    |
 | 13 | s   | S   | 3    |
 | 14 | t   | T   | 4    |
 | 15 | u   | U   | 5    |
 | 16 | v   | V   | 6    |
 | 17 | w   | W   | 7    |
 | 18 | x   | X   | 8    |
 | 19 | y   | Y   | 9    |
 | 1a | z   | Z   | :    |
 | 1b | _   | [   | L0   |
 | 1c | .   | \\  | L1   |
 | 1d | -   | ]   | L2   |
 | 1e | S1  | S0  | S0   |
 | 1f | S2  | S2  | S1   |
```

## Pages
There are three pages.
The current- and locked page is page 1 at the start of the text.

By using the commands `S1`, `S2` or `S3` you can temporarilly switch
the current page until a single character is emited, afterwards the
current page is switched back to the locked page.

By using the command `L1`, `L2` or `L3` you can change the current- and
locked page. Until another `S` or `L` command.

## ASCII
The page 3 commands `A0`, `A1`, `A2` and `A4` are used to emit a single
ASCII character. The lower 2 bits of the `A` command are used as the high
2 ASCII bits, and the next 5 bit code-unit is used for the lower 5 ASCII bits.

## Unicode
The page 3 commands `U2`, `U3`, `U4` and `U5` are used to emit a single
Unicode character. The code-unit value of the command is the number of
olowing 5-bit code units in big-endian representing a unicode code-point.

## End of text
End of text is denoted by the NUL character.
