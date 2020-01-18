// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/TrueTypeFont.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/endian.hpp"
#include <cstddef>


#define assert_or_return(x, y) if (ttauri_unlikely(!(x))) { return (y); }

namespace TTauri {

struct Fixed_buf_t {
    big_uint32_buf_t x;
    float value() const noexcept { return static_cast<float>(x.value()) / 65536.0f; }
};

struct shortFrac_buf_t {
    big_int16_buf_t x;
    float value() const noexcept { return static_cast<float>(x.value()) / 32768.0f; }
};

struct FWord_buf_t {
    big_int16_buf_t x;
    float value(float unitsPerEm) const noexcept { return static_cast<float>(x.value()) / unitsPerEm; }
};

struct FByte_buf_t {
    int8_t x;
    float value(float unitsPerEm) const noexcept { return static_cast<float>(x) / unitsPerEm; }
};

struct uFWord_buf_t {
    big_uint16_buf_t x;
    float value(float unitsPerEm) const noexcept { return static_cast<float>(x.value()) / unitsPerEm; }
};

struct CMAPFormat4 {
    big_uint16_buf_t format;
    big_uint16_buf_t length;
    big_uint16_buf_t language;
    big_uint16_buf_t segCountX2;
    big_uint16_buf_t searchRange;
    big_uint16_buf_t entrySelector;
    big_uint16_buf_t rangeShift;
};


static GlyphID searchCharacterMapFormat4(gsl::span<std::byte const> bytes, char32_t c) noexcept
{
    if (c > 0xffff) {
        // character value too high.
        return {};
    }

    size_t offset = 0;

    assert_or_return(check_placement_ptr<CMAPFormat4>(bytes, offset), {});
    let header = unsafe_make_placement_ptr<CMAPFormat4>(bytes, offset);

    let length = header->length.value();
    assert_or_return(length <= bytes.size(), {});

    let segCount = header->segCountX2.value() / 2;

    assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, segCount), {});
    let endCode = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, segCount);

    offset += sizeof(uint16_t); // reservedPad

    assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, segCount), {});
    let startCode = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, segCount);

    assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, segCount), {});
    let idDelta = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, segCount);

    // The glyphIdArray is included inside idRangeOffset.
    let idRangeOffset_count = (length - offset) / sizeof(uint16_t);
    assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, idRangeOffset_count), {});
    let idRangeOffset = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, idRangeOffset_count);

    for (uint16_t i = 0; i < segCount; i++) {
        let endCode_ = endCode[i].value();
        if (c <= endCode_) {
            let startCode_ = startCode[i].value();
            if (c >= startCode_) {
                // Found the glyph.
                let idRangeOffset_ = idRangeOffset[i].value();
                if (idRangeOffset_ == 0) {
                    // Use modulo 65536 arithmatic.
                    let u16_c = static_cast<uint16_t>(c);
                    return {idDelta[i].value() + u16_c};

                } else {
                    let charOffset = c - startCode_;
                    let glyphOffset = (idRangeOffset_ / 2) + charOffset + i;

                    assert_or_return(glyphOffset < idRangeOffset.size(), -1); 
                    let glyphIndex = idRangeOffset[glyphOffset].value();
                    if (glyphIndex == 0) {
                        return {};
                    } else {
                        // Use modulo 65536 arithmatic.
                        return {idDelta[i].value() + glyphIndex};
                    }
                }

            } else {
                // character outside of segment
                return {};
            }
        }
    }

    // Could not find character.
    return {};
}

[[nodiscard]] static UnicodeRanges parseCharacterMapFormat4(gsl::span<std::byte const> bytes)
{
    UnicodeRanges r;

    size_t offset = 0;
    let header = make_placement_ptr<CMAPFormat4>(bytes, offset);
    let length = header->length.value();
    parse_assert(length <= bytes.size());
    let segCount = header->segCountX2.value() / 2;

    let endCode = make_placement_array<big_uint16_buf_t>(bytes, offset, segCount);
    offset += sizeof(uint16_t); // reservedPad
    let startCode = make_placement_array<big_uint16_buf_t>(bytes, offset, segCount);

    for (int i = 0; i != segCount; ++i) {
        r.addCodePointRange(static_cast<char32_t>(startCode[i].value()), static_cast<char32_t>(endCode[i].value()) + 1);
    }

    return r;
}

struct CMAPFormat6 {
    big_uint16_buf_t format;
    big_uint16_buf_t length;
    big_uint16_buf_t language;
    big_uint16_buf_t firstCode;
    big_uint16_buf_t entryCount;
};

static GlyphID searchCharacterMapFormat6(gsl::span<std::byte const> bytes, char32_t c) noexcept
{
    size_t offset = 0;

    assert_or_return(check_placement_ptr<CMAPFormat6>(bytes, offset), {});
    let header = unsafe_make_placement_ptr<CMAPFormat6>(bytes, offset);

    let firstCode = static_cast<char32_t>(header->firstCode.value());
    let entryCount = header->entryCount.value();
    if (c < firstCode || c >= static_cast<char32_t>(firstCode + entryCount)) {
        // Character outside of range.
        return {};
    }

    assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, entryCount), {});
    let glyphIndexArray = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, entryCount);

    let charOffset = c - firstCode;
    assert_or_return(charOffset < glyphIndexArray.size(), {});
    return {glyphIndexArray[charOffset].value()};
}

[[nodiscard]] static UnicodeRanges parseCharacterMapFormat6(gsl::span<std::byte const> bytes)
{
    UnicodeRanges r;

    size_t offset = 0;
    let header = make_placement_ptr<CMAPFormat6>(bytes, offset);
    let firstCode = static_cast<char32_t>(header->firstCode.value());
    let entryCount = header->entryCount.value();

    r.addCodePointRange(static_cast<char32_t>(firstCode), static_cast<char32_t>(firstCode) + entryCount);

    return r;
}

struct CMAPFormat12 {
    big_uint32_buf_t format;
    big_uint32_buf_t length;
    big_uint32_buf_t language;
    big_uint32_buf_t numGroups;
};

struct CMAPFormat12Group {
    big_uint32_buf_t startCharCode;
    big_uint32_buf_t endCharCode;
    big_uint32_buf_t startGlyphID;
};

static GlyphID searchCharacterMapFormat12(gsl::span<std::byte const> bytes, char32_t c) noexcept
{
    size_t offset = 0;

    assert_or_return(check_placement_ptr<CMAPFormat12>(bytes, offset), {});
    let header = unsafe_make_placement_ptr<CMAPFormat12>(bytes, offset);

    let numGroups = header->numGroups.value();

    assert_or_return(check_placement_array<CMAPFormat12Group>(bytes, offset, numGroups), {});
    let entries = unsafe_make_placement_array<CMAPFormat12Group>(bytes, offset, numGroups);

    let i = std::lower_bound(entries.begin(), entries.end(), c, [](let &element, char32_t value) {
        return element.endCharCode.value() < value;
    });

    if (i != entries.end()) {
        let &entry = *i;
        let startCharCode = entry.startCharCode.value();
        if (c >= startCharCode) {
            c -= startCharCode;
            return {entry.startGlyphID.value() + c};
        } else {
            // Character was not in this group.
            return {};
        }

    } else {
        // Character was not in map.
        return {};
    }
}

[[nodiscard]] static UnicodeRanges parseCharacterMapFormat12(gsl::span<std::byte const> bytes)
{
    UnicodeRanges r;

    size_t offset = 0;
    let header = make_placement_ptr<CMAPFormat12>(bytes, offset);
    let numGroups = header->numGroups.value();

    let entries = make_placement_array<CMAPFormat12Group>(bytes, offset, numGroups);
    for (let &entry: entries) {
        r.addCodePointRange(static_cast<char32_t>(entry.startCharCode.value()), static_cast<char32_t>(entry.endCharCode.value()) + 1);
    }
    return r;
}

[[nodiscard]] UnicodeRanges TrueTypeFont::parseCharacterMap()
{
    let format = make_placement_ptr<big_uint16_buf_t>(cmapBytes);

    switch (format->value()) {
    case 4: return parseCharacterMapFormat4(cmapBytes);
    case 6: return parseCharacterMapFormat6(cmapBytes);
    case 12: return parseCharacterMapFormat12(cmapBytes);
    default:
        TTAURI_THROW(parse_error("Unknown character map format {}", format->value()));
    }
}

[[nodiscard]] GlyphID TrueTypeFont::getGlyph(char32_t c) const noexcept
{
    assert_or_return(check_placement_ptr<big_uint16_buf_t>(cmapBytes), {});
    let format = unsafe_make_placement_ptr<big_uint16_buf_t>(cmapBytes);

    switch (format->value()) {
    case 4: return searchCharacterMapFormat4(cmapBytes, c);
    case 6: return searchCharacterMapFormat6(cmapBytes, c);
    case 12: return searchCharacterMapFormat12(cmapBytes, c);
    default: return {};
    }
}

int TrueTypeFont::searchCharacterMap(char32_t c) const noexcept
{
    if (auto id = getGlyph(c)) {
        return static_cast<int>(id);
    } else {
        return -1;
    }
}

struct CMAPHeader {
    big_uint16_buf_t version;
    big_uint16_buf_t numTables;
};

struct CMAPEntry {
    big_uint16_buf_t platformID;
    big_uint16_buf_t platformSpecificID;
    big_uint32_buf_t offset;
};

static gsl::span<std::byte const> parseCharacterMapDirectory(gsl::span<std::byte const> bytes)
{
    size_t offset = 0;

    let header = make_placement_ptr<CMAPHeader>(bytes, offset);
    parse_assert(header->version.value() == 0);

    uint16_t numTables = header->numTables.value();
    let entries = make_placement_array<CMAPEntry>(bytes, offset, numTables);

    // Entries are ordered by platformID, then platformSpecificID.
    // This allows us to search reasonable quickly for the best entries.
    // The following order is searched: 0.4,0.3,0.2,0.1,3.10,3.1,3.0.
    CMAPEntry const *bestEntry = nullptr;
    for (let &entry: entries) {
        switch (entry.platformID.value()) {
        case 0: {
                // Unicode.
                switch (entry.platformSpecificID.value()) {
                case 0: // Default
                case 1: // Version 1.1
                case 2: // ISO 10646 1993
                case 3: // Unicode 2.0 BMP-only
                case 4: // Unicode 2.0 non-BMP
                    // The best entry is the last one.
                    bestEntry = &entry;
                    break;
                default:
                    // Not interesting
                    break;
                }
            } break;

        case 3: {
                // Microsoft Windows
                switch (entry.platformSpecificID.value()) {
                case 0: // Symbol
                case 1: // Unicode 16-bit
                case 10: // Unicode 32-bit
                    // The best entry is the last one.
                    bestEntry = &entry;
                    break;
                default:
                    // Not unicode
                    break;
                }
            } break;
        default:
            break;
        }
    }

    // There must be a bestEntry because a unicode table is required by the true-type standard.
    parse_assert(bestEntry != nullptr);

    let entry_offset = bestEntry->offset.value();
    parse_assert(entry_offset < bytes.size());

    return bytes.subspan(entry_offset, bytes.size() - entry_offset);
}

struct HHEATable {
    big_int16_buf_t majorVersion;
    big_int16_buf_t minorVersion;
    FWord_buf_t ascender;
    FWord_buf_t descender;
    FWord_buf_t lineGap;
    uFWord_buf_t advanceWidthMax;
    FWord_buf_t minLeftSideBearing;
    FWord_buf_t minRightSideBearing;
    FWord_buf_t xMaxExtent;
    big_int16_buf_t caretSlopeRise;
    big_int16_buf_t caretSlopRun;
    big_int16_buf_t caretOffset;
    big_int16_buf_t reserved0;
    big_int16_buf_t reserved1;
    big_int16_buf_t reserved2;
    big_int16_buf_t reserved3;
    big_int16_buf_t metricDataFormat;
    big_int16_buf_t numberOfHMetrics;
};

void TrueTypeFont::parseHheaTable(gsl::span<std::byte const> bytes)
{
    let table = make_placement_ptr<HHEATable>(bytes);

    parse_assert(table->majorVersion.value() == 1 && table->minorVersion.value() == 0);
    ascender = table->ascender.value(unitsPerEm);
    descender = table->descender.value(unitsPerEm);
    numberOfHMetrics = table->numberOfHMetrics.value();
}

struct HEADTable {
    big_uint16_buf_t majorVersion;
    big_uint16_buf_t minorVersion;
    Fixed_buf_t fontRevision;
    big_uint32_buf_t checkSumAdjustment;
    big_uint32_buf_t magicNumber;
    big_uint16_buf_t flags;
    big_uint16_buf_t unitsPerEm;
    big_uint64_buf_t created;
    big_uint64_buf_t modified;
    FWord_buf_t xMin;
    FWord_buf_t yMin;
    FWord_buf_t xMax;
    FWord_buf_t yMax;
    big_uint16_buf_t macStyle;
    big_uint16_buf_t lowestRecPPEM;
    big_int16_buf_t fontDirectionHint;
    big_int16_buf_t indexToLocFormat;
    big_int16_buf_t glyphDataFormat;
};

void TrueTypeFont::parseHeadTable(gsl::span<std::byte const> bytes)
{
    let table = make_placement_ptr<HEADTable>(bytes);

    parse_assert(table->majorVersion.value() == 1 && table->minorVersion.value() == 0);
    parse_assert(table->magicNumber.value() == 0x5f0f3cf5);

    let indexToLogFormat = table->indexToLocFormat.value();
    parse_assert(indexToLogFormat <= 1);
    locaTableIsOffset32 = indexToLogFormat == 1;

    unitsPerEm = table->unitsPerEm.value();
    emScale = 1.0f / unitsPerEm;
}

struct NAMETable {
    big_uint16_buf_t format;
    big_uint16_buf_t count;
    big_uint16_buf_t stringOffset;
};

struct NAMERecord {
    big_uint16_buf_t platformID;
    big_uint16_buf_t platformSpecificID;
    big_uint16_buf_t languageID;
    big_uint16_buf_t nameID;
    big_uint16_buf_t length;
    big_uint16_buf_t offset;
};

static std::optional<std::string> getStringFromNameTable(gsl::span<std::byte const> bytes, size_t offset, size_t lengthInBytes, uint16_t platformID, uint16_t platformSpecificID, uint16_t languageID)
{
    parse_assert(to_signed(offset + lengthInBytes) <= bytes.size());

    switch (platformID) {
    case 2: // Deprecated, but compatible with unicode.
        [[fallthrough]];
    case 0: // Unicode, encoded as UTF-16LE or UTF-16BE (BE is default guess).
        if (languageID == 0 || languageID == 0xffff) { // Language independent.
            let p = reinterpret_cast<char16_t const *>(bytes.data() + offset);
            let l = lengthInBytes / 2;
            parse_error(is_aligned(p));
            parse_error(lengthInBytes % 2 == 0);

            let s = std::u16string_view(p, l);
            return translateString<std::string>(s, TranslateStringOptions{}.byteSwap(endian == Endian::Little));
        }
        break;

    case 1: // Macintosh
        if (platformSpecificID == 0 && languageID == 0) { // Roman script ASCII, English
            let p = reinterpret_cast<char const *>(bytes.data() + offset);
            return std::string(p, lengthInBytes);
        }
        break;

    case 3: // Windows
        if (platformSpecificID == 1 && languageID == 0x409) { // UTF-16BE, English - United States.
            let p = reinterpret_cast<char16_t const *>(bytes.data() + offset);
            let l = lengthInBytes / 2;
            parse_error(is_aligned(p));
            parse_error(lengthInBytes % 2 == 0);

            let s = std::u16string_view(p, l);
            return translateString<std::string>(s, TranslateStringOptions{}.byteSwap(endian == Endian::Little));
        }
        break;

    default:
        break;
    }
    return {};
}

void TrueTypeFont::parseNameTable(gsl::span<std::byte const> bytes)
{
    size_t offset = 0;

    let table = make_placement_ptr<NAMETable>(bytes, offset);
    parse_assert(table->format.value() == 0 || table->format.value() == 1);
    size_t storageAreaOffset = table->stringOffset.value();

    uint16_t numRecords = table->count.value();
    let records = make_placement_array<NAMERecord>(bytes, offset, numRecords);

    bool familyIsTypographic = false;
    bool subFamilyIsTypographic = false;

    for (let &record: records) {
        let languageID = record.languageID.value();
        let platformID = record.platformID.value();
        let platformSpecificID = record.platformSpecificID.value();
        let nameOffset = storageAreaOffset + record.offset.value();
        let nameLengthInBytes = record.length.value();

        switch (record.nameID.value()) {
        case 1: { // Font family.(Only valid when used with only 4 sub-families Regular, Bold, Italic, Bold-Italic).
            if (!familyIsTypographic) {
                auto s = getStringFromNameTable(bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
                if (s) {
                    description.family_name = std::move(*s);
                }
            }
            } break;

        case 2: { // Font sub-family. (Only valid when used with only 4 sub-families Regular, Bold, Italic, Bold-Italic).
            if (!subFamilyIsTypographic) {
                auto s = getStringFromNameTable(bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
                if (s) {
                    description.sub_family_name = std::move(*s);
                }
            }
            } break;

        case 16: { // Typographic family.
            auto s = getStringFromNameTable(bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
            if (s) {
                description.family_name = std::move(*s);
                familyIsTypographic = true;
            }
            } break;

        case 17: { // Typographic sub-family.
            auto s = getStringFromNameTable(bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
            if (s) {
                description.sub_family_name = std::move(*s);
                subFamilyIsTypographic = true;
            }
            } break;

        default:
            continue;
        }
    }
}

struct PanoseTable {
    uint8_t bFamilyType;
    uint8_t bSerifStyle;
    uint8_t bWeight;
    uint8_t bProportion;
    uint8_t bContrast;
    uint8_t bStrokeVariation;
    uint8_t bArmStyle;
    uint8_t bLetterform;
    uint8_t bMidline;
    uint8_t bXHeight;
};

struct OS2Table2 {
    big_uint16_buf_t version;
    big_int16_buf_t xAvgCharWidth;
    big_uint16_buf_t usWeightClass;
    big_uint16_buf_t usWidthClass;
    big_uint16_buf_t fsType;
    big_int16_buf_t ySubscriptXSize;
    big_int16_buf_t ySubscriptYSize;
    big_int16_buf_t ySubscriptXOffset;
    big_int16_buf_t ySubscriptYOffset;
    big_int16_buf_t ySuperscriptXSize;
    big_int16_buf_t ySuperscriptYSize;
    big_int16_buf_t ySuperscriptXOffset;
    big_int16_buf_t ySuperscriptYOffset;
    big_int16_buf_t yStrikeoutSize;
    big_int16_buf_t yStrikeoutPosition;
    big_int16_buf_t sFamilyClass;
    PanoseTable panose;
    big_uint32_buf_t ulUnicodeRange1;
    big_uint32_buf_t ulUnicodeRange2;
    big_uint32_buf_t ulUnicodeRange3;
    big_uint32_buf_t ulUnicodeRange4;
    big_uint32_buf_t achVendID;
    big_uint16_buf_t fsSelection;
    big_uint16_buf_t usFirstCharIndex;
    big_uint16_buf_t usLastCharIndex;
    big_int16_buf_t sTypoAscender;
    big_int16_buf_t sTypoDescender;
    big_int16_buf_t sTypoLineGap;
    big_uint16_buf_t usWinAscent;
    big_uint16_buf_t usWinDescent;
    big_uint32_buf_t ulCodePageRange1;
    big_uint32_buf_t ulCodePageRange2;
    big_int16_buf_t sxHeight;
    big_int16_buf_t sCapHeight;
    big_uint16_buf_t usDefaultChar;
    big_uint16_buf_t usBreakChar;
    big_uint16_buf_t usMaxContext;
};

struct OS2Table0 {
    big_uint16_buf_t version;
    big_int16_buf_t xAvgCharWidth;
    big_uint16_buf_t usWeightClass;
    big_uint16_buf_t usWidthClass;
    big_uint16_buf_t fsType;
    big_int16_buf_t ySubscriptXSize;
    big_int16_buf_t ySubscriptYSize;
    big_int16_buf_t ySubscriptXOffset;
    big_int16_buf_t ySubscriptYOffset;
    big_int16_buf_t ySuperscriptXSize;
    big_int16_buf_t ySuperscriptYSize;
    big_int16_buf_t ySuperscriptXOffset;
    big_int16_buf_t ySuperscriptYOffset;
    big_int16_buf_t yStrikeoutSize;
    big_int16_buf_t yStrikeoutPosition;
    big_int16_buf_t sFamilyClass;
    PanoseTable panose;
    big_uint32_buf_t ulUnicodeRange1;
    big_uint32_buf_t ulUnicodeRange2;
    big_uint32_buf_t ulUnicodeRange3;
    big_uint32_buf_t ulUnicodeRange4;
    big_uint32_buf_t achVendID;
    big_uint16_buf_t fsSelection;
    big_uint16_buf_t usFirstCharIndex;
    big_uint16_buf_t usLastCharIndex;
    // For legacy reasons don't include the next 5 fields.
    //big_int16_buf_t sTypoAscender;
    //big_int16_buf_t sTypoDescender;
    //big_int16_buf_t sTypoLineGap;
    //big_uint16_buf_t usWinAscent;
    //big_uint16_buf_t usWinDescent;
};

void TrueTypeFont::parseOS2Table(gsl::span<std::byte const> bytes)
{
    let table = make_placement_ptr<OS2Table0>(bytes);
    let version = table->version.value();
    parse_assert(version <= 5);

    let weight_value = table->usWeightClass.value();
    if (weight_value >= 1 && weight_value < 150) {
        description.weight = font_weight::Thin;
    } else if (weight_value < 250) {
        description.weight = font_weight::ExtraLight;
    } else if (weight_value < 350) {
        description.weight = font_weight::Light;
    } else if (weight_value < 500) {
        description.weight = font_weight::Regular;
    } else if (weight_value < 650) {
        description.weight = font_weight::SemiBold;
    } else if (weight_value < 750) {
        description.weight = font_weight::Bold;
    } else if (weight_value < 875) {
        description.weight = font_weight::ExtraBold;
    } else if (weight_value <= 1000) {
        description.weight = font_weight::Black;
    }

    let width_value = table->usWidthClass.value();
    if (width_value >= 1 && width_value <= 4) {
        description.condensed = true;
    } else if (width_value >= 5 && width_value <= 9) {
        description.condensed = false;
    }

    let serif_value = table->panose.bSerifStyle;
    if ((serif_value >= 2 && serif_value <= 10) || (serif_value >= 14 && serif_value <= 15)) {
        description.serif = true;
    } else if (serif_value >= 11 || serif_value <= 13) {
        description.serif = false;
    }

    switch (table->panose.bWeight) {
    case 2: description.weight = font_weight::Thin; break;
    case 3: description.weight = font_weight::ExtraLight; break;
    case 4: description.weight = font_weight::Light; break;
    case 5: description.weight = font_weight::Regular; break;
    case 6: description.weight = font_weight::Regular; break;
    case 7: description.weight = font_weight::SemiBold; break;
    case 8: description.weight = font_weight::Bold; break;
    case 9: description.weight = font_weight::ExtraBold; break;
    case 10: description.weight = font_weight::Black; break;
    case 11: description.weight = font_weight::Black; break;
    default: break;
    }

    switch (table->panose.bProportion) {
    case 2: [[fallthrough]];
    case 3: [[fallthrough]];
    case 4: [[fallthrough]];
    case 5: [[fallthrough]];
    case 7: description.monospace = false; description.condensed = false; break;
    case 6: [[fallthrough]];
    case 8: description.monospace = false; description.condensed = true; break;
    case 9: description.monospace = true; description.condensed = false; break;
    }

    let letterform_value = table->panose.bLetterform;
    if (letterform_value >= 2 && letterform_value <= 8) {
        description.italic = false;
    } else if (letterform_value >= 9 && letterform_value <= 15) {
        description.italic = true;
    }

    description.unicode_ranges.value[0] = table->ulUnicodeRange1.value();
    description.unicode_ranges.value[1] = table->ulUnicodeRange2.value();
    description.unicode_ranges.value[2] = table->ulUnicodeRange3.value();
    description.unicode_ranges.value[3] = table->ulUnicodeRange4.value();

    if (version >= 2) {
        let table = make_placement_ptr<OS2Table2>(bytes);

        OS2_xHeight = table->sxHeight.value();
        OS2_HHeight = table->sCapHeight.value();
    }
}

struct MAXPTable05 {
    big_uint32_buf_t version;
    big_uint16_buf_t numGlyphs;
};

struct MAXPTable10 {
    big_uint32_buf_t version;
    big_uint16_buf_t numGlyphs;
    big_uint16_buf_t maxPoints;
    big_uint16_buf_t maxContours;
    big_uint16_buf_t maxComponentPoints;
    big_uint16_buf_t maxComponentContours;
    big_uint16_buf_t maxZones;
    big_uint16_buf_t maxTwilightPoints;
    big_uint16_buf_t maxStorage;
    big_uint16_buf_t maxFunctionDefs;
    big_uint16_buf_t maxInstructionDefs;
    big_uint16_buf_t maxStackElements;
    big_uint16_buf_t maxSizeOfInstructions;
    big_uint16_buf_t maxComponentElements;
    big_uint16_buf_t maxComponentDepth;
};

void TrueTypeFont::parseMaxpTable(gsl::span<std::byte const> bytes)
{
    parse_assert(sizeof(MAXPTable05) <= bytes.size());
    let table = make_placement_ptr<MAXPTable05>(bytes);

    let version = table->version.value();
    parse_assert(version == 0x00010000 || version == 0x00005000);

    numGlyphs = table->numGlyphs.value();
}

bool TrueTypeFont::getGlyphBytes(int glyphIndex, gsl::span<std::byte const> &bytes) const noexcept
{
    assert_or_return(glyphIndex >= 0 && glyphIndex < numGlyphs, false);

    size_t startOffset = 0;
    size_t endOffset = 0;
    if (locaTableIsOffset32) {
        let entries = make_placement_array<big_uint32_buf_t>(locaTableBytes);
        assert_or_return(entries.contains(glyphIndex + 1), false);

        startOffset = entries[glyphIndex].value();
        endOffset = entries[glyphIndex + 1].value();

    } else {
        let entries = make_placement_array<big_uint16_buf_t>(locaTableBytes);
        assert_or_return(entries.contains(glyphIndex + 1), false);

        startOffset = entries[glyphIndex].value() * 2;
        endOffset = entries[glyphIndex + 1].value() * 2;
    }

    assert_or_return(startOffset <= endOffset, false);
    let size = endOffset - startOffset;

    assert_or_return(endOffset <= static_cast<size_t>(glyfTableBytes.size()), false);
    bytes = glyfTableBytes.subspan(startOffset, size);
    return true;
}

struct HMTXEntry {
    uFWord_buf_t advanceWidth;
    FWord_buf_t leftSideBearing;
};

bool TrueTypeFont::updateGlyphMetrics(int glyphIndex, GlyphMetrics &metrics) const noexcept
{
    assert_or_return(glyphIndex >= 0 && glyphIndex < numGlyphs, false);

    size_t offset = 0;

    assert_or_return(check_placement_array<HMTXEntry>(hmtxTableBytes, offset, numberOfHMetrics), false);
    let longHorizontalMetricTable = unsafe_make_placement_array<HMTXEntry>(hmtxTableBytes, offset, numberOfHMetrics);

    let numberOfLeftSideBearings = numGlyphs - numberOfHMetrics;
    assert_or_return(check_placement_array<FWord_buf_t>(hmtxTableBytes, offset, numberOfLeftSideBearings), false);
    let leftSideBearings = unsafe_make_placement_array<FWord_buf_t>(hmtxTableBytes, offset, numberOfLeftSideBearings);

    float advanceWidth = 0.0f;
    float leftSideBearing;
    if (glyphIndex < numberOfHMetrics) {
        advanceWidth = longHorizontalMetricTable[glyphIndex].advanceWidth.value(unitsPerEm);
        leftSideBearing = longHorizontalMetricTable[glyphIndex].leftSideBearing.value(unitsPerEm);
    } else {
        advanceWidth = longHorizontalMetricTable[numberOfHMetrics - 1].advanceWidth.value(unitsPerEm);
        leftSideBearing = leftSideBearings[glyphIndex - numberOfHMetrics].value(unitsPerEm);
    }

    metrics.advance = glm::vec2{advanceWidth, 0.0f};
    metrics.leftSideBearing = glm::vec2{leftSideBearing, 0.0f};
    metrics.rightSideBearing = glm::vec2{advanceWidth - (leftSideBearing + metrics.boundingBox.extent.width()), 0.0f};
    metrics.ascender = glm::vec2{0.0f, ascender};
    metrics.descender = glm::vec2{0.0f, descender};
    metrics.xHeight = glm::vec2{0.0f, description.xHeight};
    metrics.capHeight = glm::vec2{0.0f, description.HHeight};
    return true;
}

struct GLYFEntry {
    big_int16_buf_t numberOfContours;
    FWord_buf_t xMin;
    FWord_buf_t yMin;
    FWord_buf_t xMax;
    FWord_buf_t yMax;
};

constexpr uint8_t FLAG_ON_CURVE = 0x01;
constexpr uint8_t FLAG_X_SHORT = 0x02;
constexpr uint8_t FLAG_Y_SHORT = 0x04;
constexpr uint8_t FLAG_REPEAT = 0x08;
constexpr uint8_t FLAG_X_SAME = 0x10;
constexpr uint8_t FLAG_Y_SAME = 0x20;
bool TrueTypeFont::loadSimpleGlyph(gsl::span<std::byte const> bytes, Path &glyph) const noexcept
{
    size_t offset = 0;

    assert_or_return(check_placement_ptr<GLYFEntry>(bytes, offset), false);
    let entry = unsafe_make_placement_ptr<GLYFEntry>(bytes, offset);

    let numberOfContours = static_cast<size_t>(entry->numberOfContours.value());

    // Check includes instructionLength.
    assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, numberOfContours), false);
    let endPoints = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, numberOfContours);

    for (let endPoint: endPoints) {
        glyph.contourEndPoints.push_back(endPoint.value());
    }

    let numberOfPoints = endPoints[numberOfContours - 1].value() + 1;

    // Skip over the instructions.
    assert_or_return(check_placement_ptr<big_uint16_buf_t>(bytes, offset), false);
    let instructionLength = unsafe_make_placement_ptr<big_uint16_buf_t>(bytes, offset)->value();
    offset += instructionLength * sizeof(uint8_t);

    // Extract all the flags.
    std::vector<uint8_t> flags;
    flags.reserve(numberOfPoints);
    while (flags.size() < numberOfPoints) {
        assert_or_return(check_placement_ptr<uint8_t>(bytes, offset), false);
        let flag = *unsafe_make_placement_ptr<uint8_t>(bytes, offset);

        flags.push_back(flag);
        if (flag & FLAG_REPEAT) {
            assert_or_return(check_placement_ptr<uint8_t>(bytes, offset), false);
            let repeat = *unsafe_make_placement_ptr<uint8_t>(bytes, offset);

            for (size_t i = 0; i < repeat; i++) {
                flags.push_back(flag);
            }
        }
    }
    assert_or_return(flags.size() == numberOfPoints, false);

    let point_table_size = std::accumulate(flags.begin(), flags.end(), static_cast<size_t>(0), [](auto size, auto flag) {
        return size +
            ((flag & FLAG_X_SHORT) > 0 ? 1 : ((flag & FLAG_X_SAME) > 0 ? 0 : 2)) +
            ((flag & FLAG_Y_SHORT) > 0 ? 1 : ((flag & FLAG_Y_SAME) > 0 ? 0 : 2));
    });
    assert_or_return(offset + point_table_size <= static_cast<size_t>(bytes.size()), false);

    // Get xCoordinates
    std::vector<int16_t> xCoordinates;
    xCoordinates.reserve(numberOfPoints);
    for (let flag: flags) {
        if ((flag & FLAG_X_SHORT) > 0) {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(static_cast<int16_t>(*make_placement_ptr<uint8_t>(bytes, offset)));
            } else {
                // Negative short.
                xCoordinates.push_back(-static_cast<int16_t>(*make_placement_ptr<uint8_t>(bytes, offset)));
            }
        } else {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(0);
            } else {
                // Long
                xCoordinates.push_back(make_placement_ptr<big_int16_buf_t>(bytes, offset)->value());
            }
        }
    }

    // Get yCoordinates
    std::vector<int16_t> yCoordinates;
    yCoordinates.reserve(numberOfPoints);
    for (let flag: flags) {
        if ((flag & FLAG_Y_SHORT) > 0) {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(static_cast<int16_t>(*make_placement_ptr<uint8_t>(bytes, offset)));
            } else {
                // Negative short.
                yCoordinates.push_back(-static_cast<int16_t>(*make_placement_ptr<uint8_t>(bytes, offset)));
            }
        } else {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(0);
            } else {
                // Long
                yCoordinates.push_back(make_placement_ptr<big_int16_buf_t>(bytes, offset)->value());
            }
        }
    }

    // Create absolute points
    int16_t x = 0;
    int16_t y = 0;
    size_t pointNr = 0;
    std::vector<BezierPoint> points;
    points.reserve(numberOfPoints);
    for (let flag : flags) {
        x += xCoordinates[pointNr];
        y += yCoordinates[pointNr];

        let type = (flag & FLAG_ON_CURVE) > 0 ?
            BezierPoint::Type::Anchor :
            BezierPoint::Type::QuadraticControl;

        glyph.points.emplace_back(
            x * emScale,
            y * emScale,
            type 
        );
        pointNr++;
    }

    return true;
}

constexpr uint16_t FLAG_ARG_1_AND_2_ARE_WORDS = 0x0001;
constexpr uint16_t FLAG_ARGS_ARE_XY_VALUES = 0x0002;
[[maybe_unused]] constexpr uint16_t FLAG_ROUND_XY_TO_GRID = 0x0004;
constexpr uint16_t FLAG_WE_HAVE_A_SCALE = 0x0008;
constexpr uint16_t FLAG_MORE_COMPONENTS = 0x0020;
constexpr uint16_t FLAG_WE_HAVE_AN_X_AND_Y_SCALE = 0x0040;
constexpr uint16_t FLAG_WE_HAVE_A_TWO_BY_TWO = 0x0080;
[[maybe_unused]] constexpr uint16_t FLAG_WE_HAVE_INSTRUCTIONS = 0x0100;
constexpr uint16_t FLAG_USE_MY_METRICS = 0x0200;
[[maybe_unused]] constexpr uint16_t FLAG_OVERLAP_COMPOUND = 0x0400;
constexpr uint16_t FLAG_SCALED_COMPONENT_OFFSET = 0x0800;
[[maybe_unused]]constexpr uint16_t FLAG_UNSCALED_COMPONENT_OFFSET = 0x1000;
bool TrueTypeFont::loadCompoundGlyph(gsl::span<std::byte const> bytes, Path &glyph, uint16_t &metricsGlyphIndex) const noexcept
{
    size_t offset = sizeof(GLYFEntry);

    uint16_t flags;
    do {
        assert_or_return(check_placement_ptr<big_uint16_buf_t>(bytes, offset), false);
        flags = unsafe_make_placement_ptr<big_uint16_buf_t>(bytes, offset)->value();

        assert_or_return(check_placement_ptr<big_uint16_buf_t>(bytes, offset), false);
        let subGlyphIndex = unsafe_make_placement_ptr<big_uint16_buf_t>(bytes, offset)->value();

        Path subGlyph;
        assert_or_return(loadGlyph(subGlyphIndex, subGlyph), false);

        glm::vec2 subGlyphOffset;
        if (flags & FLAG_ARGS_ARE_XY_VALUES) {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                assert_or_return(check_placement_array<FWord_buf_t>(bytes, offset, 2), false);
                let tmp = unsafe_make_placement_array<FWord_buf_t>(bytes, offset, 2);
                subGlyphOffset.x = tmp[0].value(unitsPerEm);
                subGlyphOffset.y = tmp[1].value(unitsPerEm);
            } else {
                assert_or_return(check_placement_array<FByte_buf_t>(bytes, offset, 2), false);
                let tmp = unsafe_make_placement_array<FByte_buf_t>(bytes, offset, 2);
                subGlyphOffset.x = tmp[0].value(unitsPerEm);
                subGlyphOffset.y = tmp[1].value(unitsPerEm);
            }
        } else {
            size_t pointNr1;
            size_t pointNr2;
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, 2), false);
                let tmp = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, 2);
                pointNr1 = tmp[0].value();
                pointNr2 = tmp[1].value();
            } else {
                assert_or_return(check_placement_array<uint8_t>(bytes, offset, 2), false);
                let tmp = unsafe_make_placement_array<uint8_t>(bytes, offset, 2);
                pointNr1 = tmp[0];
                pointNr2 = tmp[1];
            }
            // XXX Implement
            LOG_WARNING("Reading glyph from font with !FLAG_ARGS_ARE_XY_VALUES");
            return false;
        }

        // Start with an identity matrix.
        auto subGlyphScale = glm::mat2x2(1.0f);
        if (flags & FLAG_WE_HAVE_A_SCALE) {
            assert_or_return(check_placement_ptr<shortFrac_buf_t>(bytes, offset), false);
            subGlyphScale[0][0] = unsafe_make_placement_ptr<shortFrac_buf_t>(bytes, offset)->value();
            subGlyphScale[1][1] = subGlyphScale[0][0];
        } else if (flags & FLAG_WE_HAVE_AN_X_AND_Y_SCALE) {
            assert_or_return(check_placement_array<shortFrac_buf_t>(bytes, offset, 2), false);
            let tmp = unsafe_make_placement_array<shortFrac_buf_t>(bytes, offset, 2);
            subGlyphScale[0][0] = tmp[0].value();
            subGlyphScale[1][1] = tmp[1].value();
        } else if (flags & FLAG_WE_HAVE_A_TWO_BY_TWO) {
            assert_or_return(check_placement_array<shortFrac_buf_t>(bytes, offset, 4), false);
            let tmp = unsafe_make_placement_array<shortFrac_buf_t>(bytes, offset, 4);
            subGlyphScale[0][0] = tmp[0].value();
            subGlyphScale[0][1] = tmp[1].value();
            subGlyphScale[1][0] = tmp[2].value();
            subGlyphScale[1][1] = tmp[3].value();
        }

        if (flags & FLAG_SCALED_COMPONENT_OFFSET) {
            subGlyphOffset = subGlyphOffset * subGlyphScale;
        }

        if (flags & FLAG_USE_MY_METRICS) {
            metricsGlyphIndex = subGlyphIndex;
        }

        glyph += T2D(subGlyphOffset, subGlyphScale) * subGlyph;

    } while (flags & FLAG_MORE_COMPONENTS);
    // Ignore trailing instructions.

    return true;
}

bool TrueTypeFont::loadGlyph(int glyphIndex, Path &glyph) const noexcept
{
    assert_or_return(glyphIndex >= 0 && glyphIndex < numGlyphs, false);

    gsl::span<std::byte const> bytes;
    assert_or_return(getGlyphBytes(glyphIndex, bytes), false);

    auto metricsGlyphIndex = static_cast<uint16_t>(glyphIndex);

    if (bytes.size() > 0) {
        assert_or_return(check_placement_ptr<GLYFEntry>(bytes), false);
        let entry = unsafe_make_placement_ptr<GLYFEntry>(bytes);
        let numberOfContours = entry->numberOfContours.value();

        let position = glm::vec2{ entry->xMin.value(unitsPerEm), entry->yMin.value(unitsPerEm) };
        let extent = extent2{
            entry->xMax.value(unitsPerEm) - position.x,
            entry->yMax.value(unitsPerEm) - position.y
        };
        glyph.metrics.boundingBox = { position, extent };

        if (numberOfContours > 0) {
            assert_or_return(loadSimpleGlyph(bytes, glyph), false);
        } else if (numberOfContours < 0) {
            assert_or_return(loadCompoundGlyph(bytes, glyph, metricsGlyphIndex), false);
        } else {
            // Empty glyph, such as white-space ' '.
        }

    } else {
        // Empty glyph, such as white-space ' '.
    }

    return updateGlyphMetrics(metricsGlyphIndex, glyph.metrics);
}

bool TrueTypeFont::loadCompoundGlyphMetrics(gsl::span<std::byte const> bytes, uint16_t &metricsGlyphIndex) const noexcept
{
    size_t offset = sizeof(GLYFEntry);

    uint16_t flags;
    do {
        assert_or_return(check_placement_ptr<big_uint16_buf_t>(bytes, offset), false);
        flags = unsafe_make_placement_ptr<big_uint16_buf_t>(bytes, offset)->value();

        assert_or_return(check_placement_ptr<big_uint16_buf_t>(bytes, offset), false);
        let subGlyphIndex = unsafe_make_placement_ptr<big_uint16_buf_t>(bytes, offset)->value();

        if (flags & FLAG_ARGS_ARE_XY_VALUES) {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                offset += sizeof(FWord_buf_t) * 2;
            } else {
                offset += sizeof(FByte_buf_t) * 2;
            }
        } else {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                offset += sizeof(big_uint16_buf_t) * 2;
            } else {
                offset += sizeof(uint8_t) * 2;
            }
        }

        if (flags & FLAG_WE_HAVE_A_SCALE) {
            offset += sizeof(shortFrac_buf_t);
        } else if (flags & FLAG_WE_HAVE_AN_X_AND_Y_SCALE) {
            offset += sizeof(shortFrac_buf_t) * 2;
        } else if (flags & FLAG_WE_HAVE_A_TWO_BY_TWO) {
            offset += sizeof(shortFrac_buf_t) * 4;
        }

        if (flags & FLAG_USE_MY_METRICS) {
            metricsGlyphIndex = subGlyphIndex;
            return true;
        }
    } while (flags & FLAG_MORE_COMPONENTS);
    // Ignore trailing instructions.

    return true;
}

bool TrueTypeFont::loadGlyphMetrics(int glyphIndex, GlyphMetrics &metrics) const noexcept
{
    assert_or_return(glyphIndex >= 0 && glyphIndex < numGlyphs, false);

    gsl::span<std::byte const> bytes;
    assert_or_return(getGlyphBytes(glyphIndex, bytes), false);

    auto metricsGlyphIndex = static_cast<uint16_t>(glyphIndex);

    if (bytes.size() > 0) {
        assert_or_return(check_placement_ptr<GLYFEntry>(bytes), false);
        let entry = unsafe_make_placement_ptr<GLYFEntry>(bytes);
        let numberOfContours = entry->numberOfContours.value();

        let position = glm::vec2{ entry->xMin.value(unitsPerEm), entry->yMin.value(unitsPerEm) };
        let extent = extent2{
            entry->xMax.value(unitsPerEm) - position.x,
            entry->yMax.value(unitsPerEm) - position.y
        };
        metrics.boundingBox = { position, extent };

        if (numberOfContours > 0) {
            // A simple glyph does not include metrics information in the data.
        } else if (numberOfContours < 0) {
            assert_or_return(loadCompoundGlyphMetrics(bytes, metricsGlyphIndex), false);
        } else {
            // Empty glyph, such as white-space ' '.
        }

    } else {
        // Empty glyph, such as white-space ' '.
    }

    return updateGlyphMetrics(metricsGlyphIndex, metrics);
}

struct SFNTHeader {
    big_uint32_buf_t scalerType;
    big_uint16_buf_t numTables;
    big_uint16_buf_t searchRange;
    big_uint16_buf_t entrySelector;
    big_uint16_buf_t rangeShift;
};

struct SFNTEntry {
    big_uint32_buf_t tag;
    big_uint32_buf_t checkSum;
    big_uint32_buf_t offset;
    big_uint32_buf_t length;
};

void TrueTypeFont::parseFontDirectory()
{
    size_t offset = 0;
    let header = make_placement_ptr<SFNTHeader>(bytes, offset);

    if (!(header->scalerType.value() == fourcc("true") || header->scalerType.value() == 0x00010000)) {
        TTAURI_THROW(parse_error("sfnt.scalerType is not 'true' or 0x00010000"));
    }

    let entries = make_placement_array<SFNTEntry>(bytes, offset, header->numTables.value());
    for (let &entry: entries) {
        int64_t offset = entry.offset.value();
        int64_t length = entry.length.value();

        if (offset + length > to_signed(bytes.size())) {
            TTAURI_THROW(parse_error("sfnt table-entry is out of range"));
        }

        let tableBytes = bytes.subspan(entry.offset.value(), entry.length.value());
        switch (entry.tag.value()) {
        case fourcc("cmap"):
            cmapTableBytes = tableBytes;
            cmapBytes = parseCharacterMapDirectory(cmapTableBytes);
            break;
        case fourcc("glyf"):
            glyfTableBytes = tableBytes;
            break;
        case fourcc("head"):
            headTableBytes = tableBytes;
            break;
        case fourcc("hhea"):
            hheaTableBytes = tableBytes;
            break;
        case fourcc("hmtx"):
            hmtxTableBytes = tableBytes;
            break;
        case fourcc("loca"):
            locaTableBytes = tableBytes;
            break;
        case fourcc("maxp"):
            maxpTableBytes = tableBytes;
            break;
        case fourcc("name"):
            nameTableBytes = tableBytes;
            break;
        case fourcc("post"):
            postTableBytes = tableBytes;
            break;
        case fourcc("OS/2"):
            os2TableBytes = tableBytes;
            break;
        default:
            break;
        }
    }

    if (ssize(headTableBytes) > 0) {
        parseHeadTable(headTableBytes);
    }

    if (ssize(maxpTableBytes) > 0) {
        parseMaxpTable(maxpTableBytes);
    }

    if (ssize(hheaTableBytes) > 0) {
        parseHheaTable(hheaTableBytes);
    }

    if (ssize(os2TableBytes) > 0) {
        parseOS2Table(os2TableBytes);
    }

    if (ssize(nameTableBytes) > 0) {
        parseNameTable(nameTableBytes);
    }

    if (!description.unicode_ranges) {
        description.unicode_ranges = parseCharacterMap();
    }

    if (OS2_xHeight > 0) {
        description.xHeight = emScale * OS2_xHeight;
    } else {
        let xGlyphIndex = searchCharacterMap('x');
        if (xGlyphIndex > 0) {
            GlyphMetrics metrics;
            loadGlyphMetrics(xGlyphIndex, metrics);
            description.xHeight = metrics.boundingBox.extent.height();
        }
    }

    if (OS2_HHeight > 0) {
        description.HHeight = emScale * OS2_HHeight;
    } else {
        let HGlyphIndex = searchCharacterMap('H');
        if (HGlyphIndex > 0) {
            GlyphMetrics metrics;
            loadGlyphMetrics(HGlyphIndex, metrics);
            description.HHeight = metrics.boundingBox.extent.height();
        }
    }
}

}

