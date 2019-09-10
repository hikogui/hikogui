// Copyright 2019 Pokitec
// All rights reserved.

#include "TrueTypeFont.hpp"
#include "TTauri/span.hpp"
#include "TTauri/strings.hpp"
#include <boost/endian/buffers.hpp>
#include <cstddef>


#define assert_or_return(x, y) if (ttauri_unlikely(!(x))) { return (y); }

namespace TTauri::Draw {

using namespace gsl;
using namespace boost::endian;

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


static int searchCharacterMapFormat4(gsl::span<std::byte const> bytes, char32_t c) noexcept
{
    if (c > 0xffff) {
        // character value too high.
        return 0;
    }

    size_t offset = 0;

    assert_or_return(sizeof(CMAPFormat4) <= bytes.size(), -1);
    let &header = at<CMAPFormat4>(bytes, 0);
    offset += sizeof(CMAPFormat4);

    let length = header.length.value();
    assert_or_return(length <= bytes.size(), -1);

    let segCount = header.segCountX2.value() / 2;
    assert_or_return((offset + sizeof(uint16_t) + (3 * sizeof(uint16_t) * segCount)) <= bytes.size(), -1);

    let endCode = make_span<big_uint16_buf_t>(bytes, offset, segCount);
    offset += segCount * sizeof(uint16_t);
    offset += sizeof(uint16_t); // reservedPad

    let startCode = make_span<big_uint16_buf_t>(bytes, offset, segCount);
    offset += segCount * sizeof(uint16_t);

    let idDelta = make_span<big_uint16_buf_t>(bytes, offset, segCount);
    offset += segCount * sizeof(uint16_t);

    // The glyphIdArray is included inside idRangeOffset.
    let idRangeOffset_count = (length - offset) / sizeof(uint16_t);
    let idRangeOffset = make_span<big_uint16_buf_t>(bytes, offset, idRangeOffset_count);

    for (uint16_t i = 0; i < segCount; i++) {
        let endCode_ = endCode.at(i).value();
        if (c <= endCode_) {
            let startCode_ = startCode.at(i).value();
            if (c >= startCode_) {
                // Found the glyph.
                let idRangeOffset_ = idRangeOffset.at(i).value();
                if (idRangeOffset_ == 0) {
                    // Use modulo 65536 arithmatic.
                    let u16_c = static_cast<uint16_t>(c);
                    return to_int(idDelta.at(i).value() + u16_c);

                } else {
                    let charOffset = c - startCode_;
                    let glyphOffset = (idRangeOffset_ / 2) + charOffset + i;

                    assert_or_return(glyphOffset < idRangeOffset.size(), -1); 
                    let glyphIndex = idRangeOffset.at(glyphOffset).value();
                    if (glyphIndex == 0) {
                        return 0;
                    } else {
                        // Use modulo 65536 arithmatic.
                        return to_int(idDelta[i].value() + glyphIndex);
                    }
                }

            } else {
                // character outside of segment
                return 0;
            }
        }
    }

    // Could not find character.
    return 0;
}

struct CMAPFormat6 {
    big_uint16_buf_t format;
    big_uint16_buf_t length;
    big_uint16_buf_t language;
    big_uint16_buf_t firstCode;
    big_uint16_buf_t entryCount;
};

static int searchCharacterMapFormat6(gsl::span<std::byte const> bytes, char32_t c) noexcept
{
    size_t offset = 0;

    assert_or_return(sizeof(CMAPFormat6) <= bytes.size(), -1);
    let &header = at<CMAPFormat6>(bytes, 0);
    offset += sizeof(CMAPFormat6);

    let firstCode = header.firstCode.value();
    let entryCount = header.entryCount.value();
    if (c < firstCode || c >= (firstCode + entryCount)) {
        // Character outside of range.
        return 0;
    }

    assert_or_return(offset + (entryCount * sizeof(uint16_t)) <= bytes.size(), -1);
    let glyphIndexArray = make_span<big_uint16_buf_t>(bytes, offset, entryCount);

    let charOffset = c - firstCode;
    assert_or_return(charOffset < glyphIndexArray.size(), -1);
    return glyphIndexArray.at(charOffset).value();
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

static int searchCharacterMapFormat12(gsl::span<std::byte const> bytes, char32_t c) noexcept
{
    assert_or_return(sizeof(CMAPFormat12) <= bytes.size(), -1);
    let &header = at<CMAPFormat12>(bytes, 0);
    auto offset = sizeof(CMAPFormat12);

    let numGroups = header.numGroups.value();
    assert_or_return(offset + (numGroups * sizeof(CMAPFormat12Group)) <= bytes.size(), -1);

    let entries = make_span<CMAPFormat12Group>(bytes, offset, numGroups);

    let i = std::lower_bound(entries.begin(), entries.end(), c, [](let &element, char32_t value) {
        return element.endCharCode.value() < value;
    });

    if (i != entries.end()) {
        let &entry = *i;
        let startCharCode = entry.startCharCode.value();
        if (c >= startCharCode) {
            c -= startCharCode;
            return entry.startGlyphID.value() + c; 
        } else {
            // Character was not in this group.
            return 0;
        }

    } else {
        // Character was not in map.
        return 0;
    }
}

int TrueTypeFont::searchCharacterMap(char32_t c) const noexcept
{
    assert_or_return(sizeof(uint16_t) <= cmapBytes.size(), -1);
    let &format = at<big_uint16_buf_t>(cmapBytes, 0);

    switch (format.value()) {
    case 4: return searchCharacterMapFormat4(cmapBytes, c);
    case 6: return searchCharacterMapFormat6(cmapBytes, c);
    case 12: return searchCharacterMapFormat12(cmapBytes, c);
    default:
        // Unknown glyph if we can not find the character.
        return 0;
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

    parse_assert(bytes.size() >= sizeof(CMAPHeader));
    let &header = at<CMAPHeader>(bytes, offset);
    offset += sizeof(CMAPHeader);

    parse_assert(header.version.value() == 0);

    uint16_t numTables = header.numTables.value();
    parse_assert(bytes.size() >= offset + numTables * sizeof(CMAPEntry));
    let entries = make_span<CMAPEntry>(bytes, offset, header.numTables.value());

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

void TrueTypeFont::parseHHEATable(gsl::span<std::byte const> bytes)
{
    parse_assert(sizeof(HHEATable) <= bytes.size());
    let &table = at<HHEATable>(bytes, 0);

    parse_assert(table.majorVersion.value() == 1 && table.minorVersion.value() == 0);
    ascender = table.ascender.value(unitsPerEm);
    descender = table.descender.value(unitsPerEm);
    numberOfHMetrics = table.numberOfHMetrics.value();
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
    parse_assert(sizeof(HEADTable) <= bytes.size());
    let &table = at<HEADTable>(bytes, 0);

    parse_assert(table.majorVersion.value() == 1 && table.minorVersion.value() == 0);
    parse_assert(table.magicNumber.value() == 0x5f0f3cf5);

    let indexToLogFormat = table.indexToLocFormat.value();
    parse_assert(indexToLogFormat <= 1);
    locaTableIsOffset32 = indexToLogFormat == 1;

    unitsPerEm = table.unitsPerEm.value();
    emScale = 1.0f / unitsPerEm;

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
    let &table = at<MAXPTable05>(bytes, 0);

    let version = table.version.value();
    parse_assert(version == 0x00010000 || version == 0x00005000);

    numGlyphs = table.numGlyphs.value();
}

bool TrueTypeFont::getGlyphBytes(int glyphIndex, gsl::span<std::byte const> &bytes) const noexcept
{
    assert_or_return(glyphIndex >= 0 && glyphIndex < numGlyphs, false);

    size_t startOffset = 0;
    size_t endOffset = 0;
    if (locaTableIsOffset32) {
        let entries = make_span<big_uint32_buf_t>(locaTableBytes);
        assert_or_return(glyphIndex + 1 < entries.size(), false);

        startOffset = entries.at(glyphIndex).value();
        endOffset = entries.at(glyphIndex + 1).value();

    } else {
        let entries = make_span<big_uint16_buf_t>(locaTableBytes);
        assert_or_return(glyphIndex + 1 < entries.size(), false);

        startOffset = entries.at(glyphIndex).value();
        endOffset = entries.at(glyphIndex + 1).value();
    }

    assert_or_return(startOffset <= endOffset, false);
    let size = endOffset - startOffset;

    assert_or_return(endOffset <= glyfTableBytes.size(), false);
    bytes = glyfTableBytes.subspan(startOffset, size);
    return true;
}

struct HMTXEntry {
    uFWord_buf_t advanceWidth;
    FWord_buf_t leftSideBearing;
};

bool TrueTypeFont::updateGlyphMetrics(int glyphIndex, Path &glyph) const noexcept
{
    assert_or_return(glyphIndex >= 0 && glyphIndex < numGlyphs, false);

    assert_or_return(numberOfHMetrics * sizeof(HMTXEntry) <= hmtxTableBytes.size(), false);
    let longHorizontalMetricTable = make_span<HMTXEntry>(hmtxTableBytes, 0, numberOfHMetrics);
    size_t offset = numberOfHMetrics * sizeof(HMTXEntry);

    let numberOfLeftSideBearings = numGlyphs - numberOfHMetrics;
    assert_or_return(offset + numberOfLeftSideBearings * sizeof(FWord_buf_t) <= hmtxTableBytes.size(), false);
    let leftSideBearings = make_span<FWord_buf_t>(hmtxTableBytes, offset, numberOfLeftSideBearings);

    float advanceWidth = 0.0f;
    float leftSideBearing;
    if (glyphIndex < numberOfHMetrics) {
        advanceWidth = longHorizontalMetricTable.at(glyphIndex).advanceWidth.value(unitsPerEm);
        leftSideBearing = longHorizontalMetricTable.at(glyphIndex).leftSideBearing.value(unitsPerEm);
    } else {
        advanceWidth = longHorizontalMetricTable.at(numberOfHMetrics - 1).advanceWidth.value(unitsPerEm);
        leftSideBearing = leftSideBearings.at(glyphIndex - numberOfHMetrics).value(unitsPerEm);
    }

    glyph.advance = glm::vec2{advanceWidth, 0.0f};
    glyph.leftSideBearing = glm::vec2{leftSideBearing, 0.0f};
    glyph.rightSideBearing = glm::vec2{advanceWidth - (leftSideBearing + glyph.boundingBox.extent.width()), 0.0f};
    glyph.ascender = glm::vec2{0.0f, ascender};
    glyph.descender = glm::vec2{0.0f, descender};
    glyph.xHeight = glm::vec2{0.0f, xHeight};
    glyph.capHeight = glm::vec2{0.0f, HHeight};
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
    assert_or_return(sizeof(GLYFEntry) <= bytes.size(), false);
    let &entry = at<GLYFEntry>(bytes, 0);
    size_t offset = sizeof(GLYFEntry);

    let numberOfContours = static_cast<size_t>(entry.numberOfContours.value());

    // Check includes instructionLength.
    assert_or_return(offset + (numberOfContours * sizeof(uint16_t)) + sizeof(uint16_t) <= bytes.size(), false);
    let endPoints = make_span<big_uint16_buf_t>(bytes, offset, numberOfContours);
    offset += numberOfContours * sizeof(uint16_t);

    for (let endPoint: endPoints) {
        glyph.contourEndPoints.push_back(endPoint.value());
    }

    let numberOfPoints = endPoints.at(numberOfContours - 1).value() + 1;

    // Skip over the instructions.
    let instructionLength = at<big_uint16_buf_t>(bytes, offset).value();
    offset += sizeof(uint16_t) + instructionLength * sizeof(uint8_t);

    // Extract all the flags.
    std::vector<uint8_t> flags;
    flags.reserve(numberOfPoints);
    while (flags.size() < numberOfPoints) {
        assert_or_return(offset + sizeof(uint8_t) <= bytes.size(), false);
        let flag = at<uint8_t>(bytes, offset++);

        flags.push_back(flag);
        if (flag & FLAG_REPEAT) {
            assert_or_return(offset + sizeof(uint8_t) <= bytes.size(), false);
            let repeat = at<uint8_t>(bytes, offset++);

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
    assert_or_return(offset + point_table_size <= bytes.size(), false);

    // Get xCoordinates
    std::vector<int16_t> xCoordinates;
    xCoordinates.reserve(numberOfPoints);
    for (let flag: flags) {
        if ((flag & FLAG_X_SHORT) > 0) {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(static_cast<int16_t>(at<uint8_t>(bytes, offset)));
                offset += sizeof(uint8_t);
            } else {
                // Negative short.
                xCoordinates.push_back(-static_cast<int16_t>(at<uint8_t>(bytes, offset)));
                offset += sizeof(uint8_t);
            }
        } else {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(0);
            } else {
                // Long
                xCoordinates.push_back(at<big_int16_buf_t>(bytes, offset).value());
                offset += sizeof(int16_t);
            }
        }
    }

    // Get yCoordinates
    std::vector<int16_t> yCoordinates;
    yCoordinates.reserve(numberOfPoints);
    for (let flag: flags) {
        if ((flag & FLAG_Y_SHORT) > 0) {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(static_cast<int16_t>(at<uint8_t>(bytes, offset)));
                offset += sizeof(uint8_t);
            } else {
                // Negative short.
                yCoordinates.push_back(-static_cast<int16_t>(at<uint8_t>(bytes, offset)));
                offset += sizeof(uint8_t);
            }
        } else {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(0);
            } else {
                // Long
                yCoordinates.push_back(at<big_int16_buf_t>(bytes, offset).value());
                offset += sizeof(int16_t);
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
        x += xCoordinates.at(pointNr);
        y += yCoordinates.at(pointNr);

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
        assert_or_return(offset + sizeof(uint16_t) <= bytes.size(), false);
        flags = at<big_uint16_buf_t>(bytes, offset).value();
        offset += sizeof(uint16_t);

        assert_or_return(offset + sizeof(uint16_t) <= bytes.size(), false);
        let subGlyphIndex = at<big_uint16_buf_t>(bytes, offset).value();
        offset += sizeof(uint16_t);

        int r;
        Path subGlyph;
        assert_or_return(loadGlyph(subGlyphIndex, subGlyph), false);

        glm::vec2 subGlyphOffset;
        if (flags & FLAG_ARGS_ARE_XY_VALUES) {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                assert_or_return(offset + 2 * sizeof(int16_t) <= bytes.size(), false);
                subGlyphOffset.x = at<FWord_buf_t>(bytes, offset).value(unitsPerEm);
                offset += sizeof(int16_t);
                subGlyphOffset.y = at<FWord_buf_t>(bytes, offset).value(unitsPerEm);
                offset += sizeof(int16_t);
            } else {
                assert_or_return(offset + 2 * sizeof(int8_t) <= bytes.size(), false);
                subGlyphOffset.x = at<FByte_buf_t>(bytes, offset).value(unitsPerEm);
                offset += sizeof(int8_t);
                subGlyphOffset.y = at<FByte_buf_t>(bytes, offset).value(unitsPerEm);
                offset += sizeof(int8_t);
            }
        } else {
            size_t pointNr1;
            size_t pointNr2;
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                assert_or_return(offset + 2 * sizeof(int16_t) <= bytes.size(), false);
                pointNr1 = at<big_uint16_buf_t>(bytes, offset).value();
                offset += sizeof(uint16_t);
                pointNr2 = at<big_uint16_buf_t>(bytes, offset).value();
                offset += sizeof(uint16_t);
            } else {
                assert_or_return(offset + 2 * sizeof(int8_t) <= bytes.size(), false);
                pointNr1 = at<uint8_t>(bytes, offset);
                offset += sizeof(uint8_t);
                pointNr2 = at<uint8_t>(bytes, offset);
                offset += sizeof(uint8_t);
            }
            // XXX Implement
            LOG_WARNING("Reading glyph from font with !FLAG_ARGS_ARE_XY_VALUES");
            return false;
        }

        // Start with an identity matrix.
        auto subGlyphScale = glm::mat2x2(1.0f);
        if (flags & FLAG_WE_HAVE_A_SCALE) {
            assert_or_return(offset + sizeof(uint16_t) <= bytes.size(), false);
            subGlyphScale[0][0] = at<shortFrac_buf_t>(bytes, offset).value();
            subGlyphScale[1][1] = subGlyphScale[0][0];
            offset += sizeof(uint16_t);
        } else if (flags & FLAG_WE_HAVE_AN_X_AND_Y_SCALE) {
            assert_or_return(offset + 2 * sizeof(uint16_t) <= bytes.size(), false);
            subGlyphScale[0][0] = at<shortFrac_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);
            subGlyphScale[1][1] = at<shortFrac_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);
        } else if (flags & FLAG_WE_HAVE_A_TWO_BY_TWO) {
            assert_or_return(offset + 4 * sizeof(uint16_t) <= bytes.size(), false);
            subGlyphScale[0][0] = at<shortFrac_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);
            subGlyphScale[0][1] = at<shortFrac_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);
            subGlyphScale[1][0] = at<shortFrac_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);
            subGlyphScale[1][1] = at<shortFrac_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);
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
        assert_or_return(sizeof(GLYFEntry) <= bytes.size(), false);
        let &entry = at<GLYFEntry>(bytes, 0);
        let numberOfContours = entry.numberOfContours.value();

        let position = glm::vec2{ entry.xMin.value(unitsPerEm), entry.yMin.value(unitsPerEm) };
        let extent = extent2{
            entry.xMax.value(unitsPerEm) - position.x,
            entry.yMax.value(unitsPerEm) - position.y
        };
        glyph.boundingBox = { position, extent };

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

    return updateGlyphMetrics(metricsGlyphIndex, glyph);
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
    let &header = at<SFNTHeader>(bytes, 0);

    if (!(header.scalerType.value() == fourcc("true") || header.scalerType.value() == 0x00010000)) {
        TTAURI_THROW(parse_error("sfnt.scalerType is not 'true' or 0x00010000"));
    }

    let entries = make_span<SFNTEntry>(bytes, sizeof(SFNTHeader), header.numTables.value());
    for (let &entry: entries) {
        int64_t offset = entry.offset.value();
        int64_t length = entry.length.value();

        if (offset + length > to_int64(bytes.size())) {
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
            parseHeadTable(headTableBytes);
            break;
        case fourcc("hhea"):
            hheaTableBytes = tableBytes;
            parseHHEATable(hheaTableBytes);
            break;
        case fourcc("hmtx"):
            hmtxTableBytes = tableBytes;
            break;
        case fourcc("loca"):
            locaTableBytes = tableBytes;
            break;
        case fourcc("maxp"):
            maxpTableBytes = tableBytes;
            parseMaxpTable(maxpTableBytes);
            break;
        case fourcc("name"):
            nameTableBytes = tableBytes;
            break;
        case fourcc("post"):
            postTableBytes = tableBytes;
            break;
        default:
            break;
        }
    }

    let xGlyphIndex = searchCharacterMap('x');
    if (xGlyphIndex > 0) {
        Path xGlyph;
        loadGlyph(xGlyphIndex, xGlyph);
        xHeight = xGlyph.boundingBox.extent.height();
    }

    let HGlyphIndex = searchCharacterMap('H');
    if (HGlyphIndex > 0) {
        Path HGlyph;
        loadGlyph(HGlyphIndex, HGlyph);
        HHeight = HGlyph.boundingBox.extent.height();
    }

}

}

