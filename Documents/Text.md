# Text handling.

## FontBook
When the application is started a global FontBook is instanced.
During the FontBook's instantiation it will fast-parse each TrueType
ont in the operating system's font folder.

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

Each prefered font family is assigned a FamilyID.
Each font is assigned a FontID.

Each font famiy will be assign a 

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
   is availabel for that family.
 * A grapheme + FontID is looked up in the FontBook and a FontGlyphIDs is returned.
   The returned FontID may be of another font than requested if the glyphs where
   not available in the requested fonts.
 
