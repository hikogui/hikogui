// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once


namespace hi {
inline namespace v1 {

/**
@defgroup text Basic text handling.

Characters
----------
A `hi::character` contains the following information:
 - The full unicode grapheme cluster.
 - The phrasing.
 - The language.
 - The text-style.

The information is compressed into a 64 bit integer type so that it can be used
in a `std::basic_string` and `std::basic_string_view`.

### Grapheme Cluster
In linguistics, a grapheme is the smallest functional unit of a writing system. In
other words a grapheme is a character from the view point of a user.
A grapheme cluster is a set of Unicode code-points that together form a single grapheme.

A grapheme cluster consists of a base-character followed by zero or more combining marks;
combining into an accented character or an emoji.

A grapheme cluster in HikoGUI is stored as a 24 bit integer. The values 0 through 10ffff
is a single base-character code-point with zero combining marks. Values higher than this
number are an index into an append-only table of grapheme-cluster which so far have been
used by the application, which results in a maximum of about 24 Milion different clusters.

### Phrasing
The phrasing is an enum describing the way a character is semantically used in the text.
Examples of phrasings are: regular, emphesis, strong, code, abbreviation, citation, etc.

This is used to select a sub-style from a style.

### Language and Script
The language and script of the run of text this character is part of.

There are several reasons to have language stored for each character:
- It is used to select a sub-style, possibly selecting different font, size, weight
  or color which is better fitting for different scripts and languages. For example
  when mixing English with Japanese text.
- Support for mixing languages when doing spell and grammar checks
- Better pronounciation of text for mixed languages.
- Text shaping rules for ligatures, glyph substitution and positioning are language
  and script depended.

Only the 16-bit language code nees to be stored with a character. The script code
can be determine from the base code-point of the current character and surrounding
haracters using a Unicode algorithm.

### Text-theme and text-style
The text-theme how the text needs to be displayed. The text-style is selected
from a text-theme using the: phrasing, language and script.

We store the theme in the character so that during text editing we can
modify the language, phrasing and theme separately.

The text-theme is a 16 bit index into append-only table of text-theme which so
far have been used by the application, which results in a maximum of about 65000 different
text-themes.

Markup
------
String literals and text in translation files may contain markup codes to control how
text is displayed.

The markup in string literals are surrounded by square brackets '[' and ']'.
The commands are:

  Code                        | Description
 :--------------------------- |:----------------------------------------------------------------
  '[' [a-z] ']'               | Select phrasing.
  '[' [0-9]+ ']'              | Select text-theme.
  '[' <ietf language tag> ']' | Select language.
  '[' '.' ']'                 | Set phrasing, text-theme and language back to default.
  '[' '['                     | Literal '~'.

All other combination with brackets is reserved.

When converting text back to string with markup the canonical output is as follows:
 - Commands are only output when there is a change of attributes.
 - When multiple attributes change at the same time the commands are output in the order:
   + theme
   + phrasing
   + language
 - If default attributes are passed to the convertion function then the "default" command
   must be used when able, but not for the first character (no attributes changed).

### Phrasing
 
  Command | Phrasing            | Description
 :------- |:------------------- |:----------------------------------------------------------------
  `r`     | regular             | Plain text
  `e`     | emphesis            | Emphesized, accented voice (often italic)
  `s`     | strong              | Strong, louder voice (often bold)
  `c`     | code                | A piece of code (often in non-proportional font)
  `a`     | abbreviation        | An abbreviation. (may have double underline to a description)
  `b`     | bold                | Non-semantic bold
  `i`     | italic              | Non-semantic italic
  `k`     | keyboard / control  | Shown as a push button
  `h`     | highlight / mark    | Highlight text (often yellow background)
  `m`     | math                | Using math font
  `x`     | example             | Console output (Often in non-proportional bitmap-like font)
  `u`     | unarticulated       | (Often underlined)
  `q`     | quote / citation    | A quote from someone (often a swirly italic or cursive font)
  `l`     | link                | A hyper link (often blue and underlined)


### Theme

  Command        | Theme               | Description
 :-------------- |:------------------- |:----------------------------------------------------------------
  `0`            | User-interface      | Text theme to use for labels on the GUI.
  `1`            | Text-field          | Text theme to use for multi-line text fields.
  `2` - `9`      | reserved            |
  `10` - `65535` | custom              |

### Language




 */

}}

