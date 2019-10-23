

#pragma once

namespace TTauri {


class FontFamilyID {
    uint32_t value;

    /*! Get the id of a font family that looks simular like this.
     */
    FontFamilyID getVisualFallback() const noexcept;

    /*! Get the id of a font family that has a higher change of containing a glyph.
     */
    FontFamilyID getGlyphFallback() const noexcept;
};

/*! FontID.
 * A font ID should within 19 bits so that it will fit in the range of
 * non-unicode values 0x18'0000 - 0x1f'ffff.
 *
 * bit 3:0  Font Weight times 100
 * bit 4    Italic
 * bit 18:5 Family ID.
 */
class FontID {
    uint32_t value;

public:
    FontID(int weight, bool italic, int familyId) :
        value(
            static_cast<uint32_t>(weight / 100) |
            static_cast<uint32_t>(italic ? 0x10 : 0x00) |
            (static_cast<uint32_t>(familyId) << 5);
    {}

    int getWeight() const noexcept {
        return (value & 0xf) * 100;
    }

    bool isItalic() const noexcept {
        return (value & 0x10) > 0;
    }

    int getFamilyID() const noexcept {
        return value >> 5;
    }
};

class FontEntry {
    FontID id
    FontID fontFallback;
    FontID glyphFallback;
    URL resourceLocation;
    std::string name;
    Font *font;
};

struct FontFamily {
    int id;
    char const *name;
};

const FontFamily fontFamilies[] = {
    {1, "Roboto"},
    {2, "RobotoCondesed"},
    {3, "RobotoMono"},
    {4, "RobotoSlab"},
    {}
};

struct FontFamilyToFamily {
    char const *primaryName;
    char const *fallbackName;
};

const FontFamilyToFamily fontFamilyVisualFallback[] = {
    {"Arial", "Helvetica"},
    {"Roboto", "Arial"},
    {"RobotoSlab", "TimesNewRoman"},
    {}
};

const FontFamilyToFamily fontFamilyGlyphFallback[] = {

};


class FontRegistry {

public:
    FontFamilyID getFontFamilyID(string_view fontName) const noexcept;

    FontID getFontID(string_view fontName, int weight, bool italic) const noexcept;
    FontID getVisualFallback(FontID id) const noexcept;
    FontID getGlyphFallback(FontID id) const noexcept;

    Font *fontByID(FontID id);

};


}
