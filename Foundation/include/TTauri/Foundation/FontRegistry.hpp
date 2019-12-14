

#pragma once

namespace TTauri {


class FontFamilyID {
    uint32_t value;

    explicit FontFamily(uint32_t id) noexcept :
        value(id)
    {
        ttauri_assert(id <= 0x1fff); 
    }

    explicit operator uint32_t () const noexcept {
        return value;
    }
};


enum class FontWeight {
    Thin = 0,
    Light = 1,
    Regular = 2,
    Medium = 3,
    Bold = 4,
    Heavy = 5,
    Black = 6,
    ExtraBlack =7
};

enum class FontWidth {
    Compressed = 0,
    Condensed = 1,
    Basic = 2,
    Extended = 3
};

enum class FontPosition {
    Roman = 0,
    Italic = 1
};

/*! FontID.
 * A font ID should within 19 bits so that it will fit in the range of
 * non-unicode values 0x18'0000 - 0x1f'ffff.
 *
 * Following the Linotype numbering system, sort of.
 *
 * bit 2:0  Font Weight:
 *          0=Thin 100, 1=Light 300, 2=Regular 400, 3=Medium 500
 *          4=Bold 700, 5=Heavy 800, 6=Black 900, 7=Extra Black 950
 * bit 4:3  Font Width:
 *          0=Compressed, 1=Condensed, 2=Basic, 3=Extended
 * bit 5    Font Position:
 *          0=Roman, 1=Italic
 * bit 18:6 Family ID.
 */
class FontID {
    uint32_t value;

public:
    FontID(FontWeight weight, FontWidth width, FontPosition position, FontFamily family) :
        value(
            static_cast<uint32_t>(weight) |
            (static_cast<uint32_t>(width) << 3) |
            (static_cast<uint32_t>(position) << 5) |
            (static_cast<uint32_t>(family) << 6)
        )
    {
        ttauri_assert(familyId >= 0 && familyId <= 0x1fff);
    }

    FontWeight getWeight() const noexcept {
        return static_cast<FontWeight>(value & 0x7);
    }

    FontWidth getWidth() const noexcept {
        return static_cast<FontWidth>((value >> 3) & 0x3);
    }

    FontPosition getPosition() const noexcept {
        return static_cast<FontPosition>((value >> 5) & 0x1);
    }

    FontFamily getFamilyId() const noexcept {
        return FontFamily{value >> 6};
    }
};

struct FontInfo {
    FontID id;
    char32_t beginCodePoint;
    char32_t endCodePoint;

    /*! Loaded font.
    * nullptr means not loaded.
    */
    Font *font;

    /*! Location of the font.
    * Likely a resource: URL.
    */
    URL path;

    /*! Font to fallback to when this font is invalid.
     */
    FontID visualFallback;

    /*! Font to fallback to when this font does not contain the requested unicode code-point.
    */
    FontID glyphFallback;

    /*! Font does not exist or throws an error during loading.
     */
    bool invalid;
};

struct FontFamilyInfo {
    FontFamilyID id;
    std::string name;
};

const FontFamilyInfo fontFamilies[] = {
    // id, name, visual-fallback, glyph-fallback

    // Generic families.
    {   1, "Serif",      "RobotoSlab" },
    {   2, "Sans-serif", "Roboto"},
    {   3, "Monospace",  "RobotoMono"},

    // Always available families.
    {  16, "Roboto"},
    {  17, "RobotoCondensed"},
    {  18, "RobotoMono"},
    {  19, "RobotoSlab"},

    // Noto registry 1000-2000
    // Noto fonts need begin-end-code-point to quickly find a glyph in the many files.

    // Other families start at 256.
    {}
};


/*! Font Registry.
 * This class exposes functionality to:
 *  - Search for the closest available font that is selected by the user.
 *  - Find glyphs in the selected-available-font, or find the glyph in a noto font,
 *    or use the no-character glyph in the original font.
 *
 * Since the noto font set is quite large, currently zipped 1.1GByte, the user
 * should be given the option to download specific languages.
 */
class FontRegistry {

public:
    FontFamilyID getFontFamilyID(string_view fontName) const noexcept;

    FontID getFontID(string_view fontName, int weight, bool italic) const noexcept;

    /*! Find a font that matches the request font as best as possible.
     * Internally this recurses visual-fallbacks until a font is available.
     * This will need to load the font to check if it exists and is viable.
     */
    FontID getVisualFallback(FontID id) noexcept;

    /*! Find a font that can be used to get a glyph that the current font does not contain.
     * Internally this recurses glyph-fallbacks until a font is available.
     * This will need to load the font to check if it exists and is viable.
     * 
     * The caller may need to recurse until it can find a glyph in the returned font.
     */
    FontID getGlyphFallback(FontID id) noexcept;

    /*! Check if a font by id exists.
    * This will need to load the font to check if it exists and is viable.
    */
    bool fontByIDExists(FontID id);

    /*! Return and possibly load a font by id.
     */
    Font *fontByID(FontID id);

};


}
