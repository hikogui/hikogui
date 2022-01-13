// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "true_type_font.hpp"
#include "../placement.hpp"
#include "../strings.hpp"
#include "../endian.hpp"
#include "../codec/UTF.hpp"
#include "../log.hpp"
#include "../geometry/vector.hpp"
#include "../geometry/point.hpp"
#include <cstddef>
#include <span>

#define assert_or_return(x, y) \
    if (!(x)) { \
        [[unlikely]] return y; \
    }

namespace tt::inline v1 {

struct Fixed_buf_t {
    big_uint32_buf_t x;
    float value() const noexcept
    {
        return static_cast<float>(x.value()) / 65536.0f;
    }
};

struct shortFrac_buf_t {
    big_int16_buf_t x;
    float value() const noexcept
    {
        return static_cast<float>(x.value()) / 32768.0f;
    }
};

struct FWord_buf_t {
    big_int16_buf_t x;
    float value(float unitsPerEm) const noexcept
    {
        return static_cast<float>(x.value()) / unitsPerEm;
    }
};

struct FByte_buf_t {
    int8_t x;
    float value(float unitsPerEm) const noexcept
    {
        return static_cast<float>(x) / unitsPerEm;
    }
};

struct uFWord_buf_t {
    big_uint16_buf_t x;
    float value(float unitsPerEm) const noexcept
    {
        return static_cast<float>(x.value()) / unitsPerEm;
    }
};

struct CMAPHeader {
    big_uint16_buf_t version;
    big_uint16_buf_t numTables;
};

struct CMAPEntry {
    big_uint16_buf_t platformID;
    big_uint16_buf_t platformSpecificID;
    big_uint32_buf_t offset;
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

struct CMAPFormat6 {
    big_uint16_buf_t format;
    big_uint16_buf_t length;
    big_uint16_buf_t language;
    big_uint16_buf_t firstCode;
    big_uint16_buf_t entryCount;
};

struct CMAPFormat12 {
    big_uint32_buf_t format;
    big_uint32_buf_t length;
    big_uint32_buf_t language;
    big_uint32_buf_t numGroups;
};

struct CMAPFormat12Group {
    big_uint32_buf_t startCharCode;
    big_uint32_buf_t endCharCode;
    big_uint32_buf_t startglyph_id;
};

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
    // big_int16_buf_t sTypoAscender;
    // big_int16_buf_t sTypoDescender;
    // big_int16_buf_t sTypoLineGap;
    // big_uint16_buf_t usWinAscent;
    // big_uint16_buf_t usWinDescent;
};

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
    big_uint16_buf_t numberOfHMetrics;
};

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

struct KERNTable_ver0 {
    big_uint16_buf_t version;
    big_uint16_buf_t nTables;
};

struct KERNTable_ver1 {
    big_uint32_buf_t version;
    big_uint32_buf_t nTables;
};

struct KERNSubtable_ver0 {
    big_uint16_buf_t version;
    big_uint16_buf_t length;
    big_uint16_buf_t coverage;
};

struct KERNSubtable_ver1 {
    big_uint32_buf_t length;
    big_uint16_buf_t coverage;
    big_uint16_buf_t tupleIndex;
};

struct KERNFormat0 {
    big_uint16_buf_t nPairs;
    big_uint16_buf_t searchRange;
    big_uint16_buf_t entrySelector;
    big_uint16_buf_t rangeShift;
};

struct KERNFormat0_entry {
    big_uint16_buf_t left;
    big_uint16_buf_t right;
    FWord_buf_t value;
};

struct HMTXEntry {
    uFWord_buf_t advanceWidth;
    FWord_buf_t leftSideBearing;
};

struct GLYFEntry {
    big_int16_buf_t numberOfContours;
    FWord_buf_t xMin;
    FWord_buf_t yMin;
    FWord_buf_t xMax;
    FWord_buf_t yMax;
};

static std::span<std::byte const> parseCharacterMapDirectory(std::span<std::byte const> bytes)
{
    std::size_t offset = 0;

    ttlet header = make_placement_ptr<CMAPHeader>(bytes, offset);
    tt_parse_check(header->version.value() == 0, "CMAP version is not 0");

    uint16_t numTables = header->numTables.value();
    ttlet entries = make_placement_array<CMAPEntry>(bytes, offset, numTables);

    // Entries are ordered by platformID, then platformSpecificID.
    // This allows us to search reasonable quickly for the best entries.
    // The following order is searched: 0.4,0.3,0.2,0.1,3.10,3.1,3.0.
    CMAPEntry const *bestEntry = nullptr;
    for (ttlet &entry : entries) {
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
        default: break;
        }
    }

    // There must be a bestEntry because a unicode table is required by the true-type standard.
    tt_parse_check(bestEntry != nullptr, "Missing Unicode CMAP entry");

    ttlet entry_offset = bestEntry->offset.value();
    tt_parse_check(entry_offset < bytes.size(), "CMAP entry is located beyond buffer");

    return bytes.subspan(entry_offset, bytes.size() - entry_offset);
}

static glyph_id searchCharacterMapFormat4(std::span<std::byte const> bytes, char32_t c) noexcept
{
    // We are not checking for validity of the table, as this is being done in `parseCharacterMapFormat4`.

    if (c > 0xffff) {
        // character value too high.
        return {};
    }

    std::size_t offset = 0;

    tt_axiom(check_placement_ptr<CMAPFormat4>(bytes, offset));
    ttlet header = unsafe_make_placement_ptr<CMAPFormat4>(bytes, offset);

    ttlet length = header->length.value();
    tt_axiom(length <= bytes.size());

    ttlet num_segments = header->segCountX2.value() / 2;

    tt_axiom(check_placement_array<big_uint16_buf_t>(bytes, offset, num_segments));
    ttlet end_codes = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, num_segments);

    auto c16 = static_cast<uint16_t>(c);
    ttlet end_code_it = std::lower_bound(end_codes.begin(), end_codes.end(), c16);
    if (end_code_it == end_codes.end()) {
        // The character to find has a higher value than available in the table.
        return {};
    }
    ttlet segment_i = static_cast<uint16_t>(std::distance(end_codes.begin(), end_code_it));

    offset += ssizeof(uint16_t); // reservedPad

    tt_axiom(check_placement_array<big_uint16_buf_t>(bytes, offset, num_segments));
    ttlet start_codes = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, num_segments);

    ttlet start_code = start_codes[segment_i];
    if (c16 < start_code) {
        // The character to find is inside a gap in the table.
        return {};
    }

    tt_axiom(check_placement_array<big_uint16_buf_t>(bytes, offset, num_segments));
    ttlet id_deltas = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, num_segments);

    // The glyphIdArray is included inside idRangeOffset.
    ttlet id_range_offset_count = (length - offset) / ssizeof(uint16_t);
    tt_axiom(check_placement_array<big_uint16_buf_t>(bytes, offset, id_range_offset_count));
    ttlet id_range_offsets = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, id_range_offset_count);

    // Found the glyph.
    ttlet id_range_offset = id_range_offsets[segment_i].value();
    if (id_range_offset == 0) {
        // Use modulo 65536 arithmetic.
        c16 += id_deltas[segment_i].value();
        return glyph_id{c16};

    } else {
        c16 -= start_code;
        c16 += segment_i;
        c16 += id_range_offset / 2;

        tt_axiom(c16 < id_range_offsets.size());
        uint16_t glyph_index = id_range_offsets[c16].value();
        if (glyph_index == 0) {
            return {};
        } else {
            // Use modulo 65536 arithmetic.
            glyph_index += id_deltas[segment_i].value();
            return glyph_id{glyph_index};
        }
    }
}

[[nodiscard]] static unicode_mask parseCharacterMapFormat4(std::span<std::byte const> bytes)
{
    unicode_mask r;

    std::size_t offset = 0;
    ttlet header = make_placement_ptr<CMAPFormat4>(bytes, offset);
    ttlet length = header->length.value();
    tt_parse_check(length <= bytes.size(), "CMAP header length is larger than table.");
    ttlet num_segments = header->segCountX2.value() / 2;

    ttlet end_codes = make_placement_array<big_uint16_buf_t>(bytes, offset, num_segments);
    offset += ssizeof(uint16_t); // reservedPad
    ttlet start_codes = make_placement_array<big_uint16_buf_t>(bytes, offset, num_segments);

    ttlet id_deltas = make_placement_array<big_uint16_buf_t>(bytes, offset, num_segments);

    ttlet id_range_offset_count = (length - offset) / ssizeof(uint16_t);
    ttlet id_range_offsets = make_placement_array<big_uint16_buf_t>(bytes, offset, id_range_offset_count);

    for (uint16_t segment_i = 0; segment_i != num_segments; ++segment_i) {
        ttlet end_code = end_codes[segment_i].value();
        ttlet start_code = start_codes[segment_i].value();
        r.add(static_cast<char32_t>(start_code), static_cast<char32_t>(end_code) + 1);

        ttlet id_range_offset = id_range_offsets[segment_i].value();
        if (id_range_offset != 0) {
            auto c16 = end_code;
            c16 -= start_code;
            c16 += segment_i;
            c16 += id_range_offset / 2;
            tt_parse_check(c16 < id_range_offsets.size(), "id_range_offsets invalid");
        }
    }

    return r;
}

static glyph_id searchCharacterMapFormat6(std::span<std::byte const> bytes, char32_t c) noexcept
{
    std::size_t offset = 0;

    assert_or_return(check_placement_ptr<CMAPFormat6>(bytes, offset), {});
    ttlet header = unsafe_make_placement_ptr<CMAPFormat6>(bytes, offset);

    ttlet firstCode = static_cast<char32_t>(header->firstCode.value());
    ttlet entryCount = header->entryCount.value();
    if (c < firstCode || c >= static_cast<char32_t>(firstCode + entryCount)) {
        // Character outside of range.
        return {};
    }

    assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, entryCount), {});
    ttlet glyphIndexArray = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, entryCount);

    ttlet charOffset = c - firstCode;
    assert_or_return(charOffset < glyphIndexArray.size(), {});
    return glyph_id{glyphIndexArray[charOffset].value()};
}

[[nodiscard]] static unicode_mask parseCharacterMapFormat6(std::span<std::byte const> bytes)
{
    unicode_mask r;

    std::size_t offset = 0;
    ttlet header = make_placement_ptr<CMAPFormat6>(bytes, offset);
    ttlet firstCode = static_cast<char32_t>(header->firstCode.value());
    ttlet entryCount = header->entryCount.value();

    r.add(static_cast<char32_t>(firstCode), static_cast<char32_t>(firstCode) + entryCount);

    return r;
}

static glyph_id searchCharacterMapFormat12(std::span<std::byte const> bytes, char32_t c) noexcept
{
    std::size_t offset = 0;

    assert_or_return(check_placement_ptr<CMAPFormat12>(bytes, offset), {});
    ttlet header = unsafe_make_placement_ptr<CMAPFormat12>(bytes, offset);

    ttlet numGroups = header->numGroups.value();

    assert_or_return(check_placement_array<CMAPFormat12Group>(bytes, offset, numGroups), {});
    ttlet entries = unsafe_make_placement_array<CMAPFormat12Group>(bytes, offset, numGroups);

    ttlet i = std::lower_bound(entries.begin(), entries.end(), c, [](ttlet &element, char32_t value) {
        return element.endCharCode.value() < value;
    });

    if (i != entries.end()) {
        ttlet &entry = *i;
        ttlet startCharCode = entry.startCharCode.value();
        if (c >= startCharCode) {
            c -= startCharCode;
            return glyph_id{entry.startglyph_id.value() + c};
        } else {
            // Character was not in this group.
            return {};
        }

    } else {
        // Character was not in map.
        return {};
    }
}

[[nodiscard]] static unicode_mask parseCharacterMapFormat12(std::span<std::byte const> bytes)
{
    unicode_mask r;

    std::size_t offset = 0;
    ttlet header = make_placement_ptr<CMAPFormat12>(bytes, offset);
    ttlet numGroups = header->numGroups.value();

    ttlet entries = make_placement_array<CMAPFormat12Group>(bytes, offset, numGroups);
    for (ttlet &entry : entries) {
        r.add(static_cast<char32_t>(entry.startCharCode.value()), static_cast<char32_t>(entry.endCharCode.value()) + 1);
    }
    return r;
}

[[nodiscard]] unicode_mask true_type_font::parseCharacterMap()
{
    ttlet cmapTableBytes = getTableBytes("cmap");
    ttlet cmapBytes = parseCharacterMapDirectory(cmapTableBytes);
    ttlet format = make_placement_ptr<big_uint16_buf_t>(cmapBytes);

    switch (format->value()) {
    case 4: return parseCharacterMapFormat4(cmapBytes);
    case 6: return parseCharacterMapFormat6(cmapBytes);
    case 12: return parseCharacterMapFormat12(cmapBytes);
    default: throw parse_error("Unknown character map format {}", format->value());
    }
}

[[nodiscard]] glyph_id true_type_font::find_glyph(char32_t c) const noexcept
{
    ttlet cmapTableBytes = getTableBytes("cmap");
    ttlet cmapBytes = parseCharacterMapDirectory(cmapTableBytes);
    assert_or_return(check_placement_ptr<big_uint16_buf_t>(cmapBytes), {});
    ttlet format = unsafe_make_placement_ptr<big_uint16_buf_t>(cmapBytes);

    switch (format->value()) {
    case 4: return searchCharacterMapFormat4(cmapBytes, c);
    case 6: return searchCharacterMapFormat6(cmapBytes, c);
    case 12: return searchCharacterMapFormat12(cmapBytes, c);
    default: return {};
    }
}

void true_type_font::parseHheaTable(std::span<std::byte const> table_bytes)
{
    ttlet table = make_placement_ptr<HHEATable>(table_bytes);

    tt_parse_check(table->majorVersion.value() == 1 && table->minorVersion.value() == 0, "HHEA version is not 1.0");
    metrics.ascender = table->ascender.value(unitsPerEm);
    metrics.descender = -table->descender.value(unitsPerEm);
    metrics.line_gap = table->lineGap.value(unitsPerEm);
    numberOfHMetrics = table->numberOfHMetrics.value();
}

void true_type_font::parseHeadTable(std::span<std::byte const> table_bytes)
{
    ttlet table = make_placement_ptr<HEADTable>(table_bytes);

    tt_parse_check(table->majorVersion.value() == 1 && table->minorVersion.value() == 0, "HEAD version is not 1.0");
    tt_parse_check(table->magicNumber.value() == 0x5f0f3cf5, "HEAD magic is not 0x5f0f3cf5");

    ttlet indexToLocFormat = table->indexToLocFormat.value();
    tt_parse_check(indexToLocFormat <= 1, "HEAD indexToLocFormat must be 0 or 1");
    locaTableIsOffset32 = indexToLocFormat == 1;

    unitsPerEm = table->unitsPerEm.value();
    emScale = 1.0f / unitsPerEm;
}

static std::optional<std::string> getStringFromNameTable(
    std::span<std::byte const> bytes,
    std::size_t offset,
    std::size_t lengthInBytes,
    uint16_t platformID,
    uint16_t platformSpecificID,
    uint16_t languageID)
{
    tt_parse_check(offset + lengthInBytes <= size(bytes), "Requesting name at offset beyond name table");

    switch (platformID) {
    case 2: // Deprecated, but compatible with unicode.
        [[fallthrough]];
    case 0: // Unicode, encoded as UTF-16LE or UTF-16BE (BE is default guess).
        if (languageID == 0 || languageID == 0xffff) { // Language independent.
            tt_parse_check(lengthInBytes % 2 == 0, "Length in bytes of a name must be multiple of two");
            ttlet lengthInWords = lengthInBytes / 2;

            std::byte const *src = bytes.data() + offset;
            std::byte const *src_last = src + lengthInBytes;
            ttlet src_endian = guess_utf16_endianess(src, src_last, std::endian::big);

            auto name = std::u16string{};
            name.reserve(lengthInWords);

            if (src_endian == std::endian::little) {
                while (src != src_last) {
                    auto lo = *(src++);
                    auto hi = *(src++);
                    name += (static_cast<char16_t>(hi) << 8) | static_cast<char16_t>(lo);
                }
            } else {
                while (src != src_last) {
                    auto hi = *(src++);
                    auto lo = *(src++);
                    name += (static_cast<char16_t>(hi) << 8) | static_cast<char16_t>(lo);
                }
            }

            return tt::to_string(name);
        }
        break;

    case 1: // Macintosh
        if (platformSpecificID == 0 && languageID == 0) { // Roman script ASCII, English
            ttlet p = reinterpret_cast<char const *>(bytes.data() + offset);
            return std::string(p, lengthInBytes);
        }
        break;

    case 3: // Windows
        if (platformSpecificID == 1 && languageID == 0x409) { // UTF-16BE, English - United States.
            tt_parse_check(lengthInBytes % 2 == 0, "Length in bytes of a name must be multiple of two");
            ttlet lengthInWords = lengthInBytes / 2;

            std::byte const *src = bytes.data() + offset;
            std::byte const *src_last = src + lengthInBytes;

            auto name = std::u16string{};
            name.reserve(lengthInWords);
            while (src != src_last) {
                auto hi = *(src++);
                auto lo = *(src++);
                name += (static_cast<char16_t>(hi) << 8) | static_cast<char16_t>(lo);
            }

            return tt::to_string(name);
        }
        break;

    default: break;
    }
    return {};
}

void true_type_font::parseNameTable(std::span<std::byte const> table_bytes)
{
    std::size_t offset = 0;

    ttlet table = make_placement_ptr<NAMETable>(table_bytes, offset);
    tt_parse_check(table->format.value() == 0 || table->format.value() == 1, "Name table format must be 0 or 1");
    std::size_t storageAreaOffset = table->stringOffset.value();

    uint16_t numRecords = table->count.value();
    ttlet records = make_placement_array<NAMERecord>(table_bytes, offset, numRecords);

    bool familyIsTypographic = false;
    bool subFamilyIsTypographic = false;

    for (ttlet &record : records) {
        ttlet languageID = record.languageID.value();
        ttlet platformID = record.platformID.value();
        ttlet platformSpecificID = record.platformSpecificID.value();
        ttlet nameOffset = storageAreaOffset + record.offset.value();
        ttlet nameLengthInBytes = record.length.value();

        switch (record.nameID.value()) {
        case 1: { // font family.(Only valid when used with only 4 sub-families Regular, Bold, Italic, Bold-Italic).
            if (!familyIsTypographic) {
                auto s = getStringFromNameTable(
                    table_bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
                if (s) {
                    family_name = std::move(*s);
                }
            }
        } break;

        case 2: { // font sub-family. (Only valid when used with only 4 sub-families Regular, Bold, Italic, Bold-Italic).
            if (!subFamilyIsTypographic) {
                auto s = getStringFromNameTable(
                    table_bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
                if (s) {
                    sub_family_name = std::move(*s);
                }
            }
        } break;

        case 16: { // Typographic family.
            auto s =
                getStringFromNameTable(table_bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
            if (s) {
                family_name = std::move(*s);
                familyIsTypographic = true;
            }
        } break;

        case 17: { // Typographic sub-family.
            auto s =
                getStringFromNameTable(table_bytes, nameOffset, nameLengthInBytes, platformID, platformSpecificID, languageID);
            if (s) {
                sub_family_name = std::move(*s);
                subFamilyIsTypographic = true;
            }
        } break;

        default: continue;
        }
    }
}

void true_type_font::parseOS2Table(std::span<std::byte const> table_bytes)
{
    ttlet table = make_placement_ptr<OS2Table0>(table_bytes);
    ttlet version = table->version.value();
    tt_parse_check(version <= 5, "OS2 table version must be 0-5");

    ttlet weight_value = table->usWeightClass.value();
    if (weight_value >= 1 && weight_value <= 1000) {
        weight = font_weight_from_int(weight_value);
    }

    ttlet width_value = table->usWidthClass.value();
    if (width_value >= 1 && width_value <= 4) {
        condensed = true;
    } else if (width_value >= 5 && width_value <= 9) {
        condensed = false;
    }

    ttlet serif_value = table->panose.bSerifStyle;
    if ((serif_value >= 2 && serif_value <= 10) || (serif_value >= 14 && serif_value <= 15)) {
        serif = true;
    } else if (serif_value >= 11 && serif_value <= 13) {
        serif = false;
    }

    // The Panose weight table is odd, assuming the integer values are
    // increasing with boldness, Thin is bolder then Light.
    // The table below uses the integer value as an indication of boldness.
    switch (table->panose.bWeight) {
    case 2: weight = font_weight::Thin; break;
    case 3: weight = font_weight::ExtraLight; break;
    case 4: weight = font_weight::Light; break;
    case 5: weight = font_weight::Regular; break;
    case 6: weight = font_weight::Medium; break;
    case 7: weight = font_weight::SemiBold; break;
    case 8: weight = font_weight::Bold; break;
    case 9: weight = font_weight::ExtraBold; break;
    case 10: weight = font_weight::Black; break;
    case 11: weight = font_weight::ExtraBlack; break;
    default: break;
    }

    switch (table->panose.bProportion) {
    case 2: [[fallthrough]];
    case 3: [[fallthrough]];
    case 4: [[fallthrough]];
    case 5: [[fallthrough]];
    case 7:
        monospace = false;
        condensed = false;
        break;
    case 6: [[fallthrough]];
    case 8:
        monospace = false;
        condensed = true;
        break;
    case 9:
        monospace = true;
        condensed = false;
        break;
    }

    ttlet letterform_value = table->panose.bLetterform;
    if (letterform_value >= 2 && letterform_value <= 8) {
        italic = false;
    } else if (letterform_value >= 9 && letterform_value <= 15) {
        italic = true;
    }

    if (version >= 2) {
        ttlet table_v2 = make_placement_ptr<OS2Table2>(table_bytes);

        OS2_x_height = table_v2->sxHeight.value();
        OS2_cap_height = table_v2->sCapHeight.value();
    }
}

void true_type_font::parseMaxpTable(std::span<std::byte const> table_bytes)
{
    tt_parse_check(ssizeof(MAXPTable05) <= ssize(table_bytes), "MAXP table is larger than buffer");
    ttlet table = make_placement_ptr<MAXPTable05>(table_bytes);

    ttlet version = table->version.value();
    tt_parse_check(version == 0x00010000 || version == 0x00005000, "MAXP version must be 0.5 or 1.0");

    numGlyphs = table->numGlyphs.value();
}

bool true_type_font::getGlyphBytes(glyph_id glyph_id, std::span<std::byte const> &glyph_bytes) const noexcept
{
    assert_or_return(glyph_id >= 0 && glyph_id < numGlyphs, false);

    ttlet locaTableBytes = getTableBytes("loca");
    ttlet glyfTableBytes = getTableBytes("glyf");

    std::size_t startOffset = 0;
    std::size_t endOffset = 0;
    if (locaTableIsOffset32) {
        ttlet entries = make_placement_array<big_uint32_buf_t>(locaTableBytes);
        assert_or_return(entries.contains(static_cast<int>(glyph_id) + 1), false);

        startOffset = entries[glyph_id].value();
        endOffset = entries[static_cast<int>(glyph_id) + 1].value();

    } else {
        ttlet entries = make_placement_array<big_uint16_buf_t>(locaTableBytes);
        assert_or_return(entries.contains(static_cast<int>(glyph_id) + 1), false);

        startOffset = entries[glyph_id].value() * 2;
        endOffset = entries[static_cast<int>(glyph_id) + 1].value() * 2;
    }

    assert_or_return(startOffset <= endOffset, false);
    ttlet size = endOffset - startOffset;

    assert_or_return(endOffset <= static_cast<std::size_t>(glyfTableBytes.size()), false);
    glyph_bytes = glyfTableBytes.subspan(startOffset, size);
    return true;
}

static void getKerningFormat0(
    std::span<std::byte const> const &bytes,
    uint16_t coverage,
    float unitsPerEm,
    glyph_id glyph1_id,
    glyph_id glyph2_id,
    vector2 &r) noexcept
{
    std::size_t offset = 0;

    assert_or_return(check_placement_ptr<KERNFormat0>(bytes, offset), );
    ttlet formatheader = unsafe_make_placement_ptr<KERNFormat0>(bytes, offset);
    ttlet nPairs = formatheader->nPairs.value();

    assert_or_return(check_placement_array<KERNFormat0_entry>(bytes, offset, nPairs), );
    ttlet entries = unsafe_make_placement_array<KERNFormat0_entry>(bytes, offset, nPairs);

    ttlet i = std::lower_bound(entries.begin(), entries.end(), std::pair{glyph1_id, glyph2_id}, [](ttlet &a, ttlet &b) {
        if (a.left.value() == b.first) {
            return a.right.value() < b.second;
        } else {
            return a.left.value() < b.first;
        }
    });
    assert_or_return(i != entries.end(), );

    if (glyph1_id == i->left.value() && glyph2_id == i->right.value()) {
        // Writing direction is assumed horizontal.
        switch (coverage & 0xf) {
        case 0x1: r.x() = r.x() + i->value.value(unitsPerEm); break;
        case 0x3: r.x() = std::min(r.x(), i->value.value(unitsPerEm)); break;
        case 0x5: r.y() = r.y() + i->value.value(unitsPerEm); break;
        case 0x7: r.y() = std::min(r.y(), i->value.value(unitsPerEm)); break;
        // Override
        case 0x9: r.x() = i->value.value(unitsPerEm); break;
        case 0xb: r.x() = i->value.value(unitsPerEm); break;
        case 0xd: r.y() = i->value.value(unitsPerEm); break;
        case 0xf: r.y() = i->value.value(unitsPerEm); break;
        default:;
        }
    }
}

static void getKerningFormat3(
    std::span<std::byte const> const &bytes,
    uint16_t coverage,
    float unitsPerEm,
    glyph_id glyph1_id,
    glyph_id glyph2_id,
    vector2 &r) noexcept
{
    tt_not_implemented();
}

[[nodiscard]] static vector2
getKerning(std::span<std::byte const> const &bytes, float unitsPerEm, glyph_id glyph1_id, glyph_id glyph2_id) noexcept
{
    auto r = vector2{0.0f, 0.0f};
    std::size_t offset = 0;

    assert_or_return(check_placement_ptr<KERNTable_ver0>(bytes, offset), r);
    ttlet header_ver0 = unsafe_make_placement_ptr<KERNTable_ver0>(bytes, offset);
    uint32_t version = header_ver0->version.value();

    uint32_t nTables = 0;
    if (version == 0x0000) {
        nTables = header_ver0->nTables.value();

    } else {
        // Restart with version 1 table.
        offset = 0;
        assert_or_return(check_placement_ptr<KERNTable_ver1>(bytes, offset), r);
        ttlet header_ver1 = unsafe_make_placement_ptr<KERNTable_ver1>(bytes, offset);
        assert_or_return(header_ver1->version.value() == 0x00010000, r);
        nTables = header_ver1->nTables.value();
    }

    for (uint32_t subtableIndex = 0; subtableIndex != nTables; ++subtableIndex) {
        ttlet subtable_offset = offset;

        uint16_t coverage = 0;
        uint32_t length = 0;
        if (version == 0x0000) {
            assert_or_return(check_placement_ptr<KERNSubtable_ver0>(bytes, offset), r);
            ttlet subheader = unsafe_make_placement_ptr<KERNSubtable_ver0>(bytes, offset);
            coverage = subheader->coverage.value();
            length = subheader->length.value();

        } else {
            assert_or_return(check_placement_ptr<KERNSubtable_ver1>(bytes, offset), r);
            ttlet subheader = unsafe_make_placement_ptr<KERNSubtable_ver1>(bytes, offset);
            coverage = subheader->coverage.value();
            length = subheader->length.value();
        }

        switch (coverage >> 8) {
        case 0: // Pairs
            getKerningFormat0(bytes.subspan(offset), coverage, unitsPerEm, glyph1_id, glyph2_id, r);
            break;
        case 3: // Compact 2D kerning values.
            getKerningFormat3(bytes.subspan(offset), coverage, unitsPerEm, glyph1_id, glyph2_id, r);
            break;
        }

        offset = subtable_offset + length;
    }

    return r;
}

[[nodiscard]] vector2 true_type_font::get_kerning(tt::glyph_id current_glyph, tt::glyph_id next_glyph) const noexcept
{
    ttlet kernTableBytes = getTableBytes("kern");
    return getKerning(kernTableBytes, unitsPerEm, current_glyph, next_glyph);
}

bool true_type_font::update_glyph_metrics(
    tt::glyph_id glyph_id,
    tt::glyph_metrics &glyph_metrics,
    tt::glyph_id kern_glyph1_id,
    tt::glyph_id kern_glyph2_id) const noexcept
{
    assert_or_return(glyph_id >= 0 && glyph_id < numGlyphs, false);

    ssize_t offset = 0;

    ttlet hmtxTableBytes = getTableBytes("hmtx");

    assert_or_return(check_placement_array<HMTXEntry>(hmtxTableBytes, offset, numberOfHMetrics), false);
    ttlet longHorizontalMetricTable = unsafe_make_placement_array<HMTXEntry>(hmtxTableBytes, offset, numberOfHMetrics);

    ttlet numberOfLeftSideBearings = numGlyphs - numberOfHMetrics;
    assert_or_return(check_placement_array<FWord_buf_t>(hmtxTableBytes, offset, numberOfLeftSideBearings), false);
    ttlet leftSideBearings = unsafe_make_placement_array<FWord_buf_t>(hmtxTableBytes, offset, numberOfLeftSideBearings);

    float advanceWidth = 0.0f;
    float leftSideBearing;
    if (glyph_id < numberOfHMetrics) {
        advanceWidth = longHorizontalMetricTable[glyph_id].advanceWidth.value(unitsPerEm);
        leftSideBearing = longHorizontalMetricTable[glyph_id].leftSideBearing.value(unitsPerEm);
    } else {
        advanceWidth = longHorizontalMetricTable[numberOfHMetrics - 1].advanceWidth.value(unitsPerEm);
        leftSideBearing = leftSideBearings[static_cast<uint16_t>(glyph_id) - numberOfHMetrics].value(unitsPerEm);
    }

    glyph_metrics.advance = vector2{advanceWidth, 0.0f};
    glyph_metrics.left_side_bearing = leftSideBearing;
    glyph_metrics.right_side_bearing = advanceWidth - (leftSideBearing + glyph_metrics.bounding_rectangle.width());

    if (kern_glyph1_id && kern_glyph2_id) {
        ttlet kernTableBytes = getTableBytes("kern");
        glyph_metrics.advance += getKerning(kernTableBytes, unitsPerEm, kern_glyph1_id, kern_glyph2_id);
    }

    return true;
}

constexpr uint8_t FLAG_ON_CURVE = 0x01;
constexpr uint8_t FLAG_X_SHORT = 0x02;
constexpr uint8_t FLAG_Y_SHORT = 0x04;
constexpr uint8_t FLAG_REPEAT = 0x08;
constexpr uint8_t FLAG_X_SAME = 0x10;
constexpr uint8_t FLAG_Y_SAME = 0x20;
bool true_type_font::loadSimpleGlyph(std::span<std::byte const> glyph_bytes, graphic_path &glyph) const noexcept
{
    std::size_t offset = 0;

    assert_or_return(check_placement_ptr<GLYFEntry>(glyph_bytes, offset), false);
    ttlet entry = unsafe_make_placement_ptr<GLYFEntry>(glyph_bytes, offset);

    ttlet numberOfContours = static_cast<std::size_t>(entry->numberOfContours.value());

    // Check includes instructionLength.
    assert_or_return(check_placement_array<big_uint16_buf_t>(glyph_bytes, offset, numberOfContours), false);
    ttlet endPoints = unsafe_make_placement_array<big_uint16_buf_t>(glyph_bytes, offset, numberOfContours);

    for (ttlet endPoint : endPoints) {
        glyph.contourEndPoints.push_back(endPoint.value());
    }

    ttlet numberOfPoints = endPoints[numberOfContours - 1].value() + 1;

    // Skip over the instructions.
    assert_or_return(check_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset), false);
    ttlet instructionLength = unsafe_make_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset)->value();
    offset += instructionLength * ssizeof(uint8_t);

    // Extract all the flags.
    std::vector<uint8_t> flags;
    flags.reserve(numberOfPoints);
    while (flags.size() < numberOfPoints) {
        assert_or_return(check_placement_ptr<uint8_t>(glyph_bytes, offset), false);
        ttlet flag = *unsafe_make_placement_ptr<uint8_t>(glyph_bytes, offset);

        flags.push_back(flag);
        if (flag & FLAG_REPEAT) {
            assert_or_return(check_placement_ptr<uint8_t>(glyph_bytes, offset), false);
            ttlet repeat = *unsafe_make_placement_ptr<uint8_t>(glyph_bytes, offset);

            for (std::size_t i = 0; i < repeat; i++) {
                flags.push_back(flag);
            }
        }
    }
    assert_or_return(flags.size() == numberOfPoints, false);

    ttlet point_table_size = std::accumulate(flags.begin(), flags.end(), static_cast<std::size_t>(0), [](auto size, auto flag) {
        return size + ((flag & FLAG_X_SHORT) > 0 ? 1 : ((flag & FLAG_X_SAME) > 0 ? 0 : 2)) +
            ((flag & FLAG_Y_SHORT) > 0 ? 1 : ((flag & FLAG_Y_SAME) > 0 ? 0 : 2));
    });
    assert_or_return(offset + point_table_size <= static_cast<std::size_t>(glyph_bytes.size()), false);

    // Get xCoordinates
    std::vector<int16_t> xCoordinates;
    xCoordinates.reserve(numberOfPoints);
    for (ttlet flag : flags) {
        if ((flag & FLAG_X_SHORT) > 0) {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            } else {
                // Negative short.
                xCoordinates.push_back(-static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            }
        } else {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(0);
            } else {
                // Long
                xCoordinates.push_back(make_placement_ptr<big_int16_buf_t>(glyph_bytes, offset)->value());
            }
        }
    }

    // Get yCoordinates
    std::vector<int16_t> yCoordinates;
    yCoordinates.reserve(numberOfPoints);
    for (ttlet flag : flags) {
        if ((flag & FLAG_Y_SHORT) > 0) {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            } else {
                // Negative short.
                yCoordinates.push_back(-static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            }
        } else {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(0);
            } else {
                // Long
                yCoordinates.push_back(make_placement_ptr<big_int16_buf_t>(glyph_bytes, offset)->value());
            }
        }
    }

    // Create absolute points
    int16_t x = 0;
    int16_t y = 0;
    std::size_t pointNr = 0;
    std::vector<bezier_point> points;
    points.reserve(numberOfPoints);
    for (ttlet flag : flags) {
        x += xCoordinates[pointNr];
        y += yCoordinates[pointNr];

        ttlet type = (flag & FLAG_ON_CURVE) > 0 ? bezier_point::Type::Anchor : bezier_point::Type::QuadraticControl;

        glyph.points.emplace_back(x * emScale, y * emScale, type);
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
[[maybe_unused]] constexpr uint16_t FLAG_UNSCALED_COMPONENT_OFFSET = 0x1000;
bool true_type_font::loadCompoundGlyph(std::span<std::byte const> glyph_bytes, graphic_path &glyph, glyph_id &metrics_glyph_id)
    const noexcept
{
    std::size_t offset = ssizeof(GLYFEntry);

    uint16_t flags;
    do {
        assert_or_return(check_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset), false);
        flags = unsafe_make_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset)->value();

        assert_or_return(check_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset), false);
        ttlet subGlyphIndex = unsafe_make_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset)->value();

        graphic_path subGlyph;
        assert_or_return(load_glyph(glyph_id{subGlyphIndex}, subGlyph), false);

        auto subGlyphOffset = vector2{};
        if (flags & FLAG_ARGS_ARE_XY_VALUES) {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                assert_or_return(check_placement_array<FWord_buf_t>(glyph_bytes, offset, 2), false);
                ttlet tmp = unsafe_make_placement_array<FWord_buf_t>(glyph_bytes, offset, 2);
                subGlyphOffset = vector2{tmp[0].value(unitsPerEm), tmp[1].value(unitsPerEm)};
            } else {
                assert_or_return(check_placement_array<FByte_buf_t>(glyph_bytes, offset, 2), false);
                ttlet tmp = unsafe_make_placement_array<FByte_buf_t>(glyph_bytes, offset, 2);
                subGlyphOffset = vector2{tmp[0].value(unitsPerEm), tmp[1].value(unitsPerEm)};
            }
        } else {
            std::size_t pointNr1;
            std::size_t pointNr2;
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                assert_or_return(check_placement_array<big_uint16_buf_t>(glyph_bytes, offset, 2), false);
                ttlet tmp = unsafe_make_placement_array<big_uint16_buf_t>(glyph_bytes, offset, 2);
                pointNr1 = tmp[0].value();
                pointNr2 = tmp[1].value();
            } else {
                assert_or_return(check_placement_array<uint8_t>(glyph_bytes, offset, 2), false);
                ttlet tmp = unsafe_make_placement_array<uint8_t>(glyph_bytes, offset, 2);
                pointNr1 = tmp[0];
                pointNr2 = tmp[1];
            }
            // XXX Implement
            tt_log_warning("Reading glyph from font with !FLAG_ARGS_ARE_XY_VALUES");
            return false;
        }

        // Start with an identity matrix.
        auto subGlyphScale = scale2{};
        if (flags & FLAG_WE_HAVE_A_SCALE) {
            assert_or_return(check_placement_ptr<shortFrac_buf_t>(glyph_bytes, offset), false);
            subGlyphScale = scale2(unsafe_make_placement_ptr<shortFrac_buf_t>(glyph_bytes, offset)->value());

        } else if (flags & FLAG_WE_HAVE_AN_X_AND_Y_SCALE) {
            assert_or_return(check_placement_array<shortFrac_buf_t>(glyph_bytes, offset, 2), false);
            ttlet tmp = unsafe_make_placement_array<shortFrac_buf_t>(glyph_bytes, offset, 2);
            subGlyphScale = scale2(tmp[0].value(), tmp[1].value());

        } else if (flags & FLAG_WE_HAVE_A_TWO_BY_TWO) {
            assert_or_return(check_placement_array<shortFrac_buf_t>(glyph_bytes, offset, 4), false);
            ttlet tmp = unsafe_make_placement_array<shortFrac_buf_t>(glyph_bytes, offset, 4);
            tt_not_implemented();
            // subGlyphScale = mat::S(
            //    tmp[0].value(),
            //    tmp[1].value(),
            //    tmp[2].value(),
            //    tmp[3].value()
            //)
        }

        if (flags & FLAG_SCALED_COMPONENT_OFFSET) {
            subGlyphOffset = subGlyphScale * subGlyphOffset;
        }

        if (flags & FLAG_USE_MY_METRICS) {
            metrics_glyph_id = subGlyphIndex;
        }

        glyph += translate2(subGlyphOffset) * subGlyphScale * subGlyph;

    } while (flags & FLAG_MORE_COMPONENTS);
    // Ignore trailing instructions.

    return true;
}

std::optional<glyph_id> true_type_font::load_glyph(glyph_id glyph_id, graphic_path &glyph) const noexcept
{
    assert_or_return(glyph_id >= 0 && glyph_id < numGlyphs, {});

    std::span<std::byte const> glyph_bytes;
    assert_or_return(getGlyphBytes(glyph_id, glyph_bytes), {});

    auto metrics_glyph_id = glyph_id;

    if (glyph_bytes.size() > 0) {
        assert_or_return(check_placement_ptr<GLYFEntry>(glyph_bytes), {});
        ttlet entry = unsafe_make_placement_ptr<GLYFEntry>(glyph_bytes);
        ttlet numberOfContours = entry->numberOfContours.value();

        if (numberOfContours > 0) {
            assert_or_return(loadSimpleGlyph(glyph_bytes, glyph), {});
        } else if (numberOfContours < 0) {
            assert_or_return(loadCompoundGlyph(glyph_bytes, glyph, metrics_glyph_id), {});
        } else {
            // Empty glyph, such as white-space ' '.
        }

    } else {
        // Empty glyph, such as white-space ' '.
    }

    return metrics_glyph_id;
}

bool true_type_font::loadCompoundglyph_metrics(std::span<std::byte const> bytes, glyph_id &metrics_glyph_id) const noexcept
{
    std::size_t offset = ssizeof(GLYFEntry);

    uint16_t flags;
    do {
        assert_or_return(check_placement_ptr<big_uint16_buf_t>(bytes, offset), false);
        flags = unsafe_make_placement_ptr<big_uint16_buf_t>(bytes, offset)->value();

        assert_or_return(check_placement_ptr<big_uint16_buf_t>(bytes, offset), false);
        ttlet subGlyphIndex = unsafe_make_placement_ptr<big_uint16_buf_t>(bytes, offset)->value();

        if (flags & FLAG_ARGS_ARE_XY_VALUES) {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                offset += ssizeof(FWord_buf_t) * 2;
            } else {
                offset += ssizeof(FByte_buf_t) * 2;
            }
        } else {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                offset += ssizeof(big_uint16_buf_t) * 2;
            } else {
                offset += ssizeof(uint8_t) * 2;
            }
        }

        if (flags & FLAG_WE_HAVE_A_SCALE) {
            offset += ssizeof(shortFrac_buf_t);
        } else if (flags & FLAG_WE_HAVE_AN_X_AND_Y_SCALE) {
            offset += ssizeof(shortFrac_buf_t) * 2;
        } else if (flags & FLAG_WE_HAVE_A_TWO_BY_TWO) {
            offset += ssizeof(shortFrac_buf_t) * 4;
        }

        if (flags & FLAG_USE_MY_METRICS) {
            metrics_glyph_id = subGlyphIndex;
            return true;
        }
    } while (flags & FLAG_MORE_COMPONENTS);
    // Ignore trailing instructions.

    return true;
}

bool true_type_font::load_glyph_metrics(tt::glyph_id glyph_id, tt::glyph_metrics &glyph_metrics, tt::glyph_id lookahead_glyph_id)
    const noexcept
{
    assert_or_return(glyph_id >= 0 && glyph_id < numGlyphs, false);

    std::span<std::byte const> glyph_bytes;
    assert_or_return(getGlyphBytes(glyph_id, glyph_bytes), false);

    auto metricsGlyphIndex = glyph_id;

    if (glyph_bytes.size() > 0) {
        assert_or_return(check_placement_ptr<GLYFEntry>(glyph_bytes), false);
        ttlet entry = unsafe_make_placement_ptr<GLYFEntry>(glyph_bytes);
        ttlet numberOfContours = entry->numberOfContours.value();

        ttlet xyMin = point2{entry->xMin.value(unitsPerEm), entry->yMin.value(unitsPerEm)};
        ttlet xyMax = point2{entry->xMax.value(unitsPerEm), entry->yMax.value(unitsPerEm)};
        glyph_metrics.bounding_rectangle = aarectangle{xyMin, xyMax};

        if (numberOfContours > 0) {
            // A simple glyph does not include metrics information in the data.
        } else if (numberOfContours < 0) {
            assert_or_return(loadCompoundglyph_metrics(glyph_bytes, metricsGlyphIndex), false);
        } else {
            // Empty glyph, such as white-space ' '.
        }

    } else {
        // Empty glyph, such as white-space ' '.
    }

    return update_glyph_metrics(metricsGlyphIndex, glyph_metrics, glyph_id, lookahead_glyph_id);
}

[[nodiscard]] std::span<std::byte const> true_type_font::getTableBytes(char const *table_name) const
{
    if (not view) {
        tt_axiom(url);
        view = url->loadView();
        ++global_counter<"ttf:map">;
    }

    ttlet bytes = view->bytes();

    std::size_t offset = 0;
    ttlet header = make_placement_ptr<SFNTHeader>(bytes, offset);

    if (!(header->scalerType.value() == fourcc("true") || header->scalerType.value() == 0x00010000)) {
        throw parse_error("sfnt.scalerType is not 'true' or 0x00010000");
    }

    ttlet entries = make_placement_array<SFNTEntry>(bytes, offset, header->numTables.value());

    ttlet tag = fourcc(table_name);
    auto it = std::lower_bound(cbegin(entries), cend(entries), tag, [](auto const &entry, auto const &tag) {
        return entry.tag < tag;
    });

    if (it != cend(entries) and it->tag == tag) {
        return bytes.subspan(it->offset.value(), it->length.value());
    } else {
        return {};
    }
}

void true_type_font::parse_font_directory()
{
    if (auto headTableBytes = getTableBytes("head"); not headTableBytes.empty()) {
        parseHeadTable(headTableBytes);
    }

    if (auto maxpTableBytes = getTableBytes("maxp"); not maxpTableBytes.empty()) {
        parseMaxpTable(maxpTableBytes);
    }

    if (auto hheaTableBytes = getTableBytes("hhea"); not hheaTableBytes.empty()) {
        parseHheaTable(hheaTableBytes);
    }

    if (auto os2TableBytes = getTableBytes("OS/2"); not os2TableBytes.empty()) {
        parseOS2Table(os2TableBytes);
    }

    if (auto nameTableBytes = getTableBytes("name"); not nameTableBytes.empty()) {
        parseNameTable(nameTableBytes);
    }

    unicode_mask = parseCharacterMap();
    unicode_mask.optimize();
    unicode_mask.shrink_to_fit();

    if (OS2_x_height > 0) {
        metrics.x_height = emScale * OS2_x_height;
    } else {
        ttlet glyph_id = find_glyph('x');
        if (glyph_id) {
            tt::glyph_metrics glyph_metrics;
            load_glyph_metrics(glyph_id, glyph_metrics);
            metrics.x_height = glyph_metrics.bounding_rectangle.height();
        }
    }

    if (OS2_cap_height > 0) {
        metrics.cap_height = emScale * OS2_cap_height;
    } else {
        ttlet glyph_id = find_glyph('H');
        if (glyph_id) {
            tt::glyph_metrics glyph_metrics;
            load_glyph_metrics(glyph_id, glyph_metrics);
            metrics.cap_height = glyph_metrics.bounding_rectangle.height();
        }
    }

    ttlet glyph_id = find_glyph('8');
    if (glyph_id) {
        tt::glyph_metrics glyph_metrics;
        load_glyph_metrics(glyph_id, glyph_metrics);
        metrics.digit_advance = glyph_metrics.advance.x();
    }
}

} // namespace tt::inline v1
