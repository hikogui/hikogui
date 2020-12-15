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
The text shaper will handle Unicode strings together with inline text style directives.
The default text style in absent of initial text style directives is as follows:
 - Font Family GID = 1 (Sans-Serif)
 - Font Weight = 500 (Medium)
 - Font Slant = upright
 - Font size = 12
 - Text decoriation = no-decoration
 - Text color = semantic-foreground

The text style directives are encoded as follows:
 1. U+91 (PU1: Private Use)
 2. One or more alphabetical characters case sensitive denoted the parameter
 3. Zero or more values seperated using U+1F, U+1E or U+1D for the different
    levels. The values themselves are integer or decimal numbers encoded as digits
    '0' to '9' and an optional single decimal seperator '.'.
 4. U+1C (FS: File Separator) To mark the end of the style directive.


 :---------|:---------- 
  0xFDD0   | Reset to default text-style
  0xFDD1   | Font Family
  0xFDD2   | Font Variant
  0xFDD3   | Font Size
  0xFDD4   | Text Decoration
  0xFDD5   | Text Color

Below are the encodings:

  Coding                                                   | Description
 :-------------------------------------------------------- | ---------------------------
  U+91 'ff' int U+1C                                       | Font family global unique id
  U+91 'fw' int U+1C                                       | Font weight from 100 to 950.
  U+91 'fi' int U+1C                                       | Font slant: 0=Upright, 1=Italic
  U+91 'ts' int U+1C                                       | Text size
  U+91 'tc' int U+1C                                       | Text color with semantic id
  U+91 'tc' int U+1F int U+1F int (U+1F int)? U+1C         | Text color sRGBA 0-255
  U+91 'tc' float U+1F float U+1F float (U+1F float)? U+1C | Text color linear extended sRGB
  U+91 'lt' int U+1C                                       | Line type
  U+91 'lc' int U+1C                                       | Line color with semantic id
  U+91 'lc' int U+1F int U+1F int (U+1F int)? U+1C         | Line color sRGBA 0-255
  U+91 'lc' float U+1F float U+1F float (U+1F float)? U+1C | Line color linear extended sRGB


  Code | Description
 :---- |:--------------
  0    | Set text color to Foreground
  100  | Set text color to Blue (Accent)
  101  | Set text color to Green (Good)
  102  | Set text color to Indigo
  103  | Set text color to Orange (Warning)
  104  | Set text color to Pink
  105  | Set text color to Purple
  106  | Set text color to Red (Error)
  107  | Set text color to Teal
  108  | Set text color to Yellow

