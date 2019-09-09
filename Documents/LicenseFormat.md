# License Format
This document describes the format of a license-code that a customer
can enter in the application to unlock functionality.

## Encoding format.
The encoding format should be made of characters that is possible to enter
manually on a keyboard. If possible it should eliminate characters
that are not easy recognisable, for example 0=O l=I. But the encoding
should not shy away from other characters.

The following map is for a base-85 (5 characters for 32-bit) format.
The table is in the same order as ASCII, with the following modifications:

 * '+' - First character is still in correct code point, but other characters
         have been added.
 * '#' - All characters at this code point have been replaced.

```
 0  (     1  )     2  *     3  +     4+ ,.    5+ -_   6- !    7  /
 8+ 0Oo   9+ 1Il  10  2    11  3    12  4    13  5   14  6   15  7
16  8    17  9    18+ :;   19- &    20  <    21  =   22  >   23  ?
24  @    25  A    26  B    27  C    28  D    29  E   30  F   31  G
32  H    33- #    34  J    35  K    36  L    37  M   38  N   39- $
40  P    41  Q    42  R    43  S    44  T    45  U   46  V   47  W
48  X    49  Y    50  Z    51  [    52  \    53  ]   54  ^   55- %
56+ `'"  57  a    58  b    59  c    60  d    61  e   62  f   63  g
64  h    65  i    66  j    67  k    68- }    69  m   70  n   71- ~
72  p    73  q    74  r    75  s    76  t    77  u   78  v   79  w
80  x    81  y    82  z    83  {    84  |

```


Base 64 alphabet

```
ABCDEFGH.JKLMN:PQRSTUVWXYZ
        ,     ;

abcdefghijk*mn!pqrstuvwxyz

0123456789+/
OI
ol
```
