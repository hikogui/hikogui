HikoGUI Rich Text
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

 --------------- | --- | ------------------
  Regular        | ~r | The default, neutral phrasing
  Emphesis       | ~e | Text with stressed emphesis. Spoken with clear articulation. Often shown in italic.
  Strong         | ~s | Text with more importance, warning, urgend. Spoken louder. Often shown in bold.
  Code           | ~c | A (sub-)expression  of computer-language-code, Often shown in a non-proportional font.
  Abbreviation   | ~a | An abbreviation, shown in a different style to show there is a definition somehwere.
  Italic         | ~i | Show text in italic, without the text beging "emphesis"
  Bold           | ~b | Show text in bold, without the text being "strong"
  Citation       | ~q | A citation of a title, or a quote.
  Keyboard       | ~k | Used in help messages to show which key to press. 
  Mark           | ~h | Used to highlight text, like using a physical yellow marker.
  Math           | ~m | A mathematical (sub-)expression. Often shown in a special italic math font.
  Unarticulated  | ~u | Unarticulated text. Often shown as regular underlined.


Rich Text format
----------------


EBNF:

```

document := part*

part := text | command | escape

text := /[^~]+/

escape := escape-open | escape-close
escape-open := '[' '['
escape-close := ']' ']'

command := phrasing | load-style | store-style | sub-style | color | font

language := '[' 'L' <ietf-language-tag> ']'

load-style := '[' 'S' name ']'
store-style := '[' 'S' '+' name ']'
sub-style := '[' 'S' ':' phrasing-mask? ( ':' <iso-language>? ( ':' <iso-script> )? )? ']'

color := '[' 'C' float ':' float ':' float ']'
color := '[' 'C' '#' hex hex hex hex hex hex ']'

font := '[' 'F' name ':' variant ']'

phrasing := '[' phrasing-char ']'

phrasing-mask := phrasing-char+

phrasing-char := 'r' | 'e' | 's' | 'c' | 'a' | 'i' | 'b' | 'q' | 'k' | 'h' | 'm' | 'u'

dec := '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
hex := dec | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'A' | 'B' | 'C' | 'D' | 'E' | 'F'

```


 ----------------------- | ------------------------
  \\l{language}          | Select ISO-639 language.
  \\f{font-family-name}



