HikoGUI Aux Text
=================

Grapheme Cluster
----------------

Text shaping and editing is done on a per-grapheme-cluster basis,
graphemes clusters are defined in "UAX #29 Unicode Text Segmentation".

Inside Hikogui a `grapheme` can be used in a `std::basic_string`. A grapheme
is 21 bits, where grapheme clusters larger than one code-point get a temporary and unique-id
assigned to the invalid unicode planes 17-31.

Text Style
----------

A text-style is actually a set of styles for different text-categories, languages
and scripts. The text-category is a mask, the language may be empty meaning a whildcard;
and script may be 'zzzz' meaning a whildcard. This allows for at least one of the sub-styles
to be a fallback.

 - filter:
   + language code
   + script code
   + phrasing mask
 - font family id
 - font variant (weight/italic)
 - font size
 - color
 - lines (under / over / strike throught).

Attributed Grapheme Cluster
---------------------------

 - language-code 15
 - country-code 10
 - phrasing 4
 - grapheme 21
 - text-style 15.

 - [63:48] Language code.
 - [47:36] Text style id.
 - [35:32] Text category.
 - [31:0] Grapheme cluster.

### Phrasing

  Name           | Cmd | Description
 --------------- | --- | -------------------------------------------------------------------------------------
  Abbreviation   | a   | An abbreviation, shown in a different style to show there is a definition somehwere.
  Bold           | b   | Show text in bold, without the text being "strong"
  Code           | c   | A (sub-)expression  of computer-language-code, Often shown in a non-proportional font.
  Emphesis       | e   | Text with stressed emphesis. Spoken with clear articulation. Often shown in italic.
  Mark           | h   | Used to highlight text, like using a physical yellow marker.
  Italic         | i   | Show text in italic, without the text beging "emphesis"
  Keyboard       | k   | Used in help messages to show which key to press. 
  Link           | l   | A link
  Math           | m   | A mathematical (sub-)expression. Often shown in a special italic math font.
  Citation       | q   | A citation of a title, or a quote.
  Regular        | r   | The default, neutral phrasing
  Strong         | s   | Text with more importance, warning, urgend. Spoken louder. Often shown in bold.
  Unarticulated  | u   | Unarticulated text. Often shown as regular underlined.


Aux Text format
----------------

### State machine

The state-machine has the following variables:
 - text-style
 - phrasing
 - language
 - font-size
 - font (selected using font-family, weight and italic-flag)
 - color
 - set of masks



### Format EBNF

```

document := part*

part := text | commands | escape-open | escape-close

text := /[^[]+/

// Emits a single '['
escape-open := '[' '['

// Emits a single ']'
escape-close := ']' ']'

// A set of commands. The ';' is only needed when a command ends in a name.
commands := '[' command (';'? command)* ']'
command := phrasing | load-style | store-style | sub-style | color | font

// Switch the phrasing of the text.
// This will set the text-style matching the filter from the loaded style.
phrasing := phrasing-abbreviation | phrasing-bold | phrasing-code |
            phrasing-emphesis | phrasing-help | phrasing-italic |
            phrasing-key | phrasing-link | phrasing-math |
            phrasing-quote | phrasing-regular | phrasing-strong |
            phrasing-unarticulated
phrasing-abbreviation := 'a'
phrasing-bold := 'b'
phrasing-code := 'c'
phrasing-emphesis := 'e'
phrasing-help := 'h'
phrasing-italic := 'i'
phrasing-key := 'k'
phrasing-link := 'l'
phrasing-math := 'm'
phrasing-quote := 'q'
phrasing-regular := 'r'
phrasing-strong := 's'
phrasing-unarticulated := 'u'

// Switch the language of the text.
// This will set the text-style matching the filter from the loaded style.
language := 'L' <ietf-language-tag>

// Load a text style with the following name.
// This will set the text-style based on the current phrasing and language.
// This will clear all the to-be-stored filters.
// ';' is needed if this command is followed by another command
read-style := 'R' style-name
style-name := name

// Store a text-style with all the to-be-stored filters.
// This will clear all the to-be-stored filters.
write-style := 'W' style-name

// Add a mask for the current: font, color, size.
add-filter := 'M' ( phrasing+ | * ) ',' ( iso-language | * ) ',' ( iso-script | *)
iso-language := name
iso-script := name

// Change the current color of the text.
color := 'C' ( color-name | float ',' float ',' float (',' float) | hex{6} )
color-name := name

// Change the current font of the text.
font := 'F' font-variant font-name
font-name := name
font-variant := 'i'? font-weight
font-weight := weight-thin | weight-extra-light | weight-light | weight-regular |
               weight-medium | weight-semi-bold | weight-bold | weight-extra-bold |
               weight-black | weight-extra-black
weight-thin := '1'
weight-extra-light := '2'
weight-light := '3'
weight-regular := '4'
weight-medium := '5'
weight-semi-bold := '6'
weight-bold := '7'
weight-extra-bold := '8'
weight-black := '9'
weight-extra-black := '0'

// Change font-size.
size := 'S' integer

name := [^]]+
integer := [0-9]+
hex := [0-9a-zA-Z]
float := [0-9]* '.' [0-9]*
```

Examples
--------

Switch for emphasis phrasing to regular phrasing.
```
foo[e]bar[r]baz
```

Read the "label" text-style, then switch to bold phrasing
```
foo[Rlabel;b]bar[r]baz
```

Select medium weight "Times New Roman" font, Select 12 pt, Set a mask for regular phrasing for all languages.
Write the "test' text-style all current masks, then delete all masks.
```
[F5Times New Roman;S12Mr:*:*Wtest]foo
```

