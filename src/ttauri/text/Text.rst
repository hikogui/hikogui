# Text handling.

## FontBook initialization
When the application is started a global FontBook is instanced.
During the FontBook's instantiation it will fast-parse each TrueType
font in the operating system's font folder.

During the fast-parse the 'name', 'OS/2' and optionally other tables
are read and then the font files are closed and memory is freed. This
fast-parse is designed to be very fast to reduce impact on application
startup time.

The following information gleamed during the fast-parse:
 * 'name' Prefered Font family name; code-16 if available, otherwise code-1
 * 'name' Prefered Font subfamily name: code-17 if available, otherwide code-2
 * 'OS/2' Font weight
 * 'OS/2' Regular/Italic
 * 'OS/2' Serif/Sans-serif
 * 'OS/2' Variable/Monospace
 * 'OS/2' Regular/Condensed
 * 'OS/2' Unicode ranges (as backup the 'cmap' table may be parsed)
 * 'OS/2' The height of 'x' and 'H' (as back the 'glyf' table may be parsed)

Each font family is assigned a FamilyID.
Each font is assigned a FontID.

There is a list of fallback font families for well known fonts, in case certain
font families are not available on the system.

Each font will be assigned a list of fallback fonts for missing glyph lookup.
Priority is given for fonts that start with the same font family name. For
example "Arial Arabic" will be prioritized when the current font is "Arial"

## Font selection
A FontVariant consists of a FontWeight+serif-flag. This allows a user to select a font
family to draw a text with and emphesize fragments of the text using italic and bold.

There are a total of maximum 20 FontVariants for each FontFamilyID.

Selecting a font to render a text is done in several steps:
 * A user select a font family name and a FontVariant.
 * The font family name is looked up in the FontBook and a FontFamilyID is
   returned for a font that exists on the system which closely matches the
   requested family.
 * The FontFamilyID + FontVariant is looked up in the FontBook and a FontID
   is returned for the variant the closely matches the requested variant that
   is available for that family.
 * A grapheme + FontID is looked up in the FontBook and a FontGlyphIDs is returned.
   The returned FontID may be of another font than requested if the glyphs where
   not available in the requested fonts.

## Rich text
The text shaper will handle Unicode strings together with a default text style.
The Unicode string may also contain special codes to change the text style inside the text.

The special codes are non-character 0xFDD0 - 0xFDEF. Non-characters are application
pecific private codes that should not be used outside the application.

  1st Code | Description
 :---------|:---------- 
  0xFDD0   | Reset to default text-style
  0xFDD1   | Font Family
  0xFDD2   | Font Variant
  0xFDD3   | Font Size
  0xFDD4   | Text Decoration
  0xFDD5   | Text Color

### Font Variant

  2nd Code | Description
 :---------|:--------------
  0xFDE0   | Reset to default variant
  0xFDE1   | Set font weight to Thin (100)
  0xFDE2   | Set font weight to ExtraLight (200)
  0xFDE3   | Set font weight to Light (300)
  0xFDE4   | Set font weight to Regular (400)
  0xFDE5   | Set font weight to Medium (500)
  0xFDE6   | Set font weight to SemiBold (600)
  0xFDE7   | Set font weight to Bold (700)
  0xFDE8   | Set font weight to ExtraBold (800)
  0xFDE9   | Set font weight to Black (900)
  0xFDEA   | Set font weight to ExtraBlack (950)
  0xFDEB   | reserved
  0xFDEC   | reserved
  0xFDED   | reserved
  0xFDEE   | Switch to italic
  0xFDEF   | Switch to upright


### Font Size

  2nd Code | Description
 :---------|:--------------
  0xFDE0   | Reset to default font size
  0xFDE1   | Set font size to 8
  0xFDE2   | Set font size to 9
  0xFDE3   | Set font size to 10
  0xFDE4   | Set font size to 11
  0xFDE5   | Set font size to 12
  0xFDE6   | Set font size to 14
  0xFDE7   | Set font size to 16
  0xFDE8   | Set font size to 18
  0xFDE9   | Set font size to 20
  0xFDEA   | Set font size to 24
  0xFDEB   | Set font size to 28
  0xFDEC   | Set font size to 32
  0xFDED   | Set font size to 40
  0xFDEE   | Set font size to 50
  0xFDEF   | Set font size to 60

### Font Family

  2nd Code | Description
 :---------|:--------------
  0xFDE0   | Reset to default font family
  0xFDE1   | Set family to Sans Serif
  0xFDE2   | Set family to Serif
  0xFDE3   | Set family to Mono space
  0xFDE4   | Set family to Arial / Helvetica
  0xFDE5   | Set family to Times New Roman / Times
  0xFDE6   | Set family to Courier New / Courier
  0xFDE7   | Set family to Palatino
  0xFDE8   | Set family to Garamond
  0xFDE9   | Set family to Bookman
  0xFDEA   | Set family to Avant Garde
  0xFDEB   | Set family to Verdana
  0xFDEC   | Set family to Georgia
  0xFDED   | Set family to Comic Sans
  0xFDEE   | Set family to Trebuchet
  0xFDEF   | Set family to Impact

### Text Decoration

  2nd Code | Description
 :---------|:--------------
  0xFDE0   | Reset to default text decoration
  0xFDE1   | 
  0xFDE2   | 
  0xFDE3   | 
  0xFDE4   | 
  0xFDE5   | 
  0xFDE6   | 
  0xFDE7   | 
  0xFDE8   | 
  0xFDE9   | 
  0xFDEA   | 
  0xFDEB   | 
  0xFDEC   | 
  0xFDED   | 
  0xFDEE   | 
  0xFDEF   | 

### Text Color

  2nd Code | Description
 :---------|:--------------
  0xFDE0   | Reset to default text color
  0xFDE1   | Set text color to Foreground
  0xFDE2   | Set text color to Accent
  0xFDE3   | Set text color to Error
  0xFDE4   | Set text color to Warning
  0xFDE5   | Set text color to Help
  0xFDE6   | Set text color to 
  0xFDE7   | Set text color to Blue
  0xFDE8   | Set text color to Green
  0xFDE9   | Set text color to Indigo
  0xFDEA   | Set text color to Orange
  0xFDEB   | Set text color to Pink
  0xFDEC   | Set text color to Purple
  0xFDED   | Set text color to Red
  0xFDEE   | Set text color to Teal
  0xFDEF   | Set text color to Yellow

